/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerLauncher.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 17:01:19 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/04/01 17:30:05 by migumore         ###   ########.fr       */
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
		serverLauncherInstance->~ServerLauncher();
	// kill(0, SIGTERM);
	std::exit(0);
}

ServerLauncher::ServerLauncher(void) : _epoll(), _sessionManager() 
{
	serverLauncherInstance = this;
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, handleSIGINT);
}

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

ServerLauncher::~ServerLauncher()
{
	std::set<Server*> deleted;
	for (std::map<int, Server*>::iterator it = _servers.begin(); it != _servers.end(); ++it)
	{
		if (deleted.insert(it->second).second)
			delete it->second;
		close(it->first);
	}
	_servers.clear();
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		delete it->second;
		close(it->first);
	}
	_clients.clear();
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
		throw std::runtime_error(e.what());
	}
	parsedConfigFile.printConfig();
	std::vector<ConfigFileServer> serverConfigs = parsedConfigFile.getServers();

	for (size_t i = 0; i < serverConfigs.size(); ++i)
	{
		Server* server = new Server(serverConfigs[i]);
		try
		{
			std::cout << "[INFO] Launching server: " << serverConfigs[i].getServerName() << std::endl;
			server->sockets();
			server->addSocketsToEpoll(_epoll);
			const std::vector<int> &serverSockets = server->getSockets();
			for (size_t j = 0; j < serverSockets.size(); ++j)
				_servers[serverSockets[j]] = server;
		}
		catch (const std::exception &e)
		{
			delete server;
			throw std::runtime_error(std::string("[ERROR] Failed to launch server: ") + e.what());
		}
	}
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
				removeClient(fd);
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
						if (_clients[fd]->keepAlive())
						{
							// std::cerr << "[DEBUG] KEEP ALIVE client FD: " << fd << std::endl;
							_epoll.modifyFD(fd, EPOLLIN);
						}
						else
							removeClient(fd);
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
		// std::cerr << "[DEBUG] NEW client FD: " << clientFd << std::endl;
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
		client->handleRequest(http, this);
		if (client->hasPendingData())
			_epoll.modifyFD(clientFd, EPOLLOUT);
		else
			_epoll.modifyFD(clientFd, EPOLLIN);
	}
	catch (const std::exception &e)
	{
		//std::cerr << "[ERROR] Client error: " << e.what() << std::endl;
		removeClient(clientFd);
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
			if (hostPorts[i].first == host && hostPorts[i].second == port)
			{
				// std::cout << "[DEBUG] Found server for host: " << host << " and port: " << port << std::endl;
				return (it->second);
			}
		}
	}
	// std::cout << "[DEBUG] No server found for host: " << host << " and port: " << port << std::endl;
	return (NULL);
}

void ServerLauncher::removeClient(int clientFd)
{
	// std::cerr << "[DEBUG] Attempting to remove client FD: " << clientFd << std::endl;
	_epoll.removeFD(clientFd);
	if (_clients.find(clientFd) != _clients.end())
	{
		// std::cerr << "[DEBUG] Removing client FD: " << clientFd << std::endl;
		delete _clients[clientFd];
		_clients.erase(clientFd);
	}
	close(clientFd);
}

void ServerLauncher::cleanupChild()
{
	std::set<Server*> deleted;
	for (std::map<int, Server*>::iterator it = _servers.begin(); it != _servers.end(); ++it)
	{
		if (deleted.insert(it->second).second)
			delete it->second;
		close(it->first);
	}
	_servers.clear();

	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		delete it->second;
		close(it->first);
	}
	_clients.clear();
	_epoll.~EPoll();
	_sessionManager.~SessionManagement();
}
