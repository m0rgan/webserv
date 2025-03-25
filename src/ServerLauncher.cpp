/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerLauncher.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 17:01:19 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/23 13:52:59 by gabrielfern      ###   ########.fr       */
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

ServerLauncher::ServerLauncher(void) : _epoll(), _sessionManager() {};

ServerLauncher::ServerLauncher(ServerLauncher const &src)
{
	(void)src;
    throw std::runtime_error("Copy constructor is not allowed for ServerLauncher");
}

ServerLauncher &ServerLauncher::operator=(ServerLauncher const &rhs)
{
	(void)rhs;
    throw std::runtime_error("Assignment operator is not allowed for ServerLauncher");
}

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
		parsedConfigFile.parser(configFile);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl; //is this neccesary since there is one in main already?
		return;
	}
	parsedConfigFile.printConfig();
	std::vector<ConfigFileServer> serverConfigs = parsedConfigFile.getServers();

	for (size_t i = 0; i < serverConfigs.size(); ++i)
	{
		try
		{
			std::cout << "[INFO] Launching server: " << serverConfigs[i].getServerName() << std::endl;
			Server* server = new Server(serverConfigs[i]);
			if (server->sockets() == 0)
			{
				server->addSocketsToEpoll(_epoll);
				const std::vector<pollfd> &serverSockets = server->getSockets();
				for (size_t j = 0; j < serverSockets.size(); ++j)
					_servers[serverSockets[j].fd] = server;
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
		int epollEvents = _epoll.wait();
		for (int i = 0; i < epollEvents; ++i)
		{
			struct epoll_event epoll = _epoll.getEvent(i);
			int fd = epoll.data.fd;

			if (epoll.events & (EPOLLHUP | EPOLLERR)) //| EPOLLNVAL
			{
				closeClient(fd);
				continue;
			}
			if (epoll.events & EPOLLIN)
			{
				if (_servers.find(fd) != _servers.end())
					newClient(fd);
				else
					existingClient(fd);
			}
			if (epoll.events & EPOLLOUT)
			{
				if (_clients.find(fd) != _clients.end())
				{
					if (_clients[fd]->hasPendingData())
					{
						_clients[fd]->writeResponse();
						if (_clients[fd]->getSocket() == -1)// || request.isEmpty())
							continue;
						if (_clients[fd]->keepAlive())
						{
							_epoll.modifyFD(fd, EPOLLIN);
							_clients[fd]->resetState();
						}
						else
							closeClient(fd);
					}
				}
			}
		}
	}
}

// The event bitmasks in events and revents have the following bits:
//      POLLPRI        High priority data may be read without blocking.
//      POLLWRBAND     Priority data may be written without blocking.

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
		setCloexecFlag(clientFd);
		_epoll.addFD(clientFd, EPOLLIN | EPOLLOUT);
		_clients[clientFd] = new Client(clientFd, server->getConfig(), _sessionManager);
	}
}

void ServerLauncher::existingClient(int clientFd)
{
	Client* client = _clients[clientFd];

	if (!client)
		return;
	try
	{
		HTTPRequest http = client->readRequest();
		
		// std::cout << "[DEBUG] Host header: " << http.getHost() << std::endl;
		if (client->getSocket() == -1)
			return;
		Server* server = serverSelector(http);
		// if (server)
		// {
		//     std::cout << "[DEBUG] Server name: " << server->getConfig().getServerName() << std::endl;
		//     std::cout << "[DEBUG] Client server name: " << client->getConfigFileServer().getServerName() << std::endl;
		// }
		if (server && server->getConfig().getServerName() != client->getConfigFileServer().getServerName())
		{
			// std::cout << "[DEBUG] Changing server config for client FD: " << clientFd << std::endl;
			client->setConfigFileServer(server->getConfig());
		}
		client->handleRequest(http);
		if (client->hasPendingData())
			_epoll.modifyFD(clientFd, EPOLLOUT);
		else
			_epoll.modifyFD(clientFd, EPOLLIN);
	}
	catch (const std::exception &e)
	{
		std::cerr << "[ERROR] Client error: " << e.what() << std::endl;
		closeClient(clientFd);
	}
}

// 	Server Block Selection Rules
// Nginx first looks for a server block with a matching listen directive and server_name.
// If multiple blocks match, it picks the first one defined in the config.
// If no server_name matches, it uses the first server block that matches the listen port.
// If multiple blocks listen on the same port but with different server_name, Nginx will default to the first one in order.

Server* ServerLauncher::serverSelector(const HTTPRequest &http)
{
	std::string host = http.getHost();
	int port = http.getPort();

	// std::cout << "[DEBUG] Looking for server for host: " << host << " and port: " << port << std::endl;
	for (std::map<int, Server*>::iterator it = _servers.begin(); it != _servers.end(); ++it)
	{
		const ConfigFileServer& config = it->second->getConfig();
		const std::vector<std::pair<std::string, int> >& hostPorts = config.getHostPort();
		for (size_t i = 0; i < hostPorts.size(); ++i)
		{
			if (config.getServerName() == host && hostPorts[i].second == port) //hostPorts[i].first == host??
			{
				// std::cout << "[DEBUG] Found server for host: " << host << " and port: " << port << std::endl;
				return (it->second);
			}
		}
	}
	// std::cout << "[DEBUG] No server found for host: " << host << " and port: " << port << std::endl;
	return (NULL);
}

void ServerLauncher::closeClient(int clientFd)
{
	_epoll.removeFD(clientFd);
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
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		delete it->second;
	_clients.clear();
}
