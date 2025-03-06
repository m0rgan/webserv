/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerLauncher.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 17:01:19 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/06 20:35:28 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ServerLauncher.hpp>
#include <ctime>
#include <iomanip>
#include <sstream>

static ServerLauncher* serverLauncherInstance = NULL;

void handleSIGINT(int sig)
{
	std::cerr << "[SIGNAL] Caught signal " << sig << " - Shutting down server" << std::endl;
	if (serverLauncherInstance)
		serverLauncherInstance->stopServers();
	kill(0, SIGTERM);
}

ServerLauncher::ServerLauncher(void) {}; //fix

ServerLauncher::ServerLauncher(const std::string &configFile)
{
	serverLauncherInstance = this;
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, handleSIGINT);

	initServers(configFile);
	loop();
}

void ServerLauncher::initServers(const std::string &configFile)
{
	ConfigFile parsedConfigFile;
	try
	{
		parsedConfigFile.process(configFile);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return;
	}
	parsedConfigFile.printConfig();
	std::vector<ServerConfig> configs = parsedConfigFile.getServers();

	for (size_t i = 0; i < configs.size(); ++i)
	{
		try
		{
			std::cout << "[INFO] Launching server: " << configs[i].getName() << std::endl;
			Server* server = new Server(configs[i]);
			//handle new error?
			if (server->sockets() == 0)
			{
				server->addSocketsToEpoll(_epoll);
				const std::vector<pollfd> &serverSockets = server->getSockets();
				for (size_t j = 0; j < serverSockets.size(); ++j)
				{
					int serverFd = serverSockets[j].fd;
					if (_servers.find(serverFd) != _servers.end())
					{
						std::cerr << "[ERROR] Failed to bind and listen on " << serverFd << std::endl;
						delete server;
						return;
					}
					_servers[serverFd] = server;
				}
			}
			else
				delete server;
		}
		catch (const std::exception &e)
		{
			std::cerr << "[ERROR] Failed to launch server: " << e.what() << std::endl;
		}
	}
}

ServerLauncher::~ServerLauncher()
{
	stopServers();
}

void ServerLauncher::loop()
{
	for (;;)
	{
		int numEvents = _epoll.wait();
		for (int i = 0; i < numEvents; ++i)
		{
			struct epoll_event event = _epoll.getEvent(i);
			int fd = event.data.fd;

			if (event.events & EPOLLIN)
			{
				if (_servers.find(fd) != _servers.end())
					newClient(fd);
				else
					existingClient(fd);
			}
			if (event.events & EPOLLOUT) //epollet and?
			{
				if (_clients.find(fd) != _clients.end())
					if (_clients[fd]->hasPendingData())
					{
						_clients[fd]->writeResponse();
						if (_clients[fd]->getSocket() == -1)// || request.isEmpty())
							continue;
						if (_clients[fd]->keepAlive())
							_epoll.modifyFd(fd, EPOLLIN);
						else
							closeClient(fd);
					}
			}
		}
	}
}

void ServerLauncher::newClient(int serverFd)
{
	Server* server = _servers[serverFd];

	if (!server)
	{
		std::cerr << "[ERROR] No server found for FD: " << serverFd << std::endl;
		return;
	}

	int clientFd = server->acceptClient(serverFd);
	if (clientFd > 0)
	{
		_epoll.addFd(clientFd, EPOLLIN | EPOLLOUT);
		_clients[clientFd] = new Client(clientFd, server->getConfig());
	}
}

// The event bitmasks in events and revents have the following bits:
//      POLLERR        An exceptional condition has occurred on the device or socket.  This flag is output
//                     only, and ignored if present in the input events bitmask.
//      POLLHUP        The device or socket has been disconnected.  This flag is output only, and ignored
//                     if present in the input events bitmask.  Note that POLLHUP and POLLOUT are mutually
//                     exclusive and should never be present in the revents bitmask at the same time.
//      POLLNVAL       The file descriptor is not open.  This flag is output only, and ignored if present
//                     in the input events bitmask.
//      POLLPRI        High priority data may be read without blocking.
//      POLLWRBAND     Priority data may be written without blocking.

void ServerLauncher::existingClient(int clientFd)
{
	Client* client = _clients[clientFd];

	if (!client)
		return;
	try
	{
		HTTPRequest http = client->readRequest();
		if (client->getSocket() == -1)
			return;
		client->handleRequest(http);
		if (client->hasPendingData())
			_epoll.modifyFd(clientFd, EPOLLOUT);
		else
			_epoll.modifyFd(clientFd, EPOLLIN);
	}
	catch (const std::exception &e)
	{
		std::cerr << "[ERROR] Client error: " << e.what() << std::endl;
		closeClient(clientFd);
	}
}

void ServerLauncher::closeClient(int clientFd)
{
	_epoll.removeFd(clientFd);

	if (_clients.find(clientFd) != _clients.end())
	{
		delete _clients[clientFd];
		_clients.erase(clientFd);
	}

	close(clientFd);
}

void ServerLauncher::stopServers()
{
	for (std::map<int, Server*>::iterator it = _servers.begin(); it != _servers.end(); ++it)
		delete it->second;
	_servers.clear();
}
