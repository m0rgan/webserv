/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerLauncher.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 17:01:19 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/05/10 17:40:28 by migumore         ###   ########.fr       */
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

			_serverConfigOrder.push_back(server); 
		}
		catch (const std::exception &e)
		{
			delete server;
			throw std::runtime_error(std::string("[ERROR] Failed to launch server: ") + e.what());
		}
	}
}

EPoll &ServerLauncher::getEpoll() { return _epoll; }

void ServerLauncher::loop()
{
	for (;;)
	{
		for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
			Client* client = it->second;
			if (client->getCGI() && std::time(NULL) - client->getCGITime() > 35) {
				client->cleanupCGIState(504);
				removeClient(client->getSocket());
			}
		}

		int epollEvents = _epoll.wait();

		for (int i = 0; i < epollEvents; ++i)
		{
			struct epoll_event epoll = _epoll.getEvent(i);
			int fd = epoll.data.fd;

			if (epoll.events & (EPOLLERR))
			{
				if (_cgiFDMap.find(fd) != _cgiFDMap.end())
				{
					int clientFd = _cgiFDMap[fd];
					Client* client = _clients[clientFd];
					if (client)
						client->cleanupCGIState(500);
					removeClient(clientFd);
				}
				continue;
			}
			if (epoll.events & (EPOLLIN | EPOLLHUP))
			{
				if (_servers.find(fd) != _servers.end())
				{
					newClient(fd);
				}
				else if (_cgiFDMap.find(fd) != _cgiFDMap.end())
				{
					int clientFd = _cgiFDMap[fd];
					Client* client = _clients[clientFd];
					if (client)
						client->handleCGIOutput(fd);
				}
				else if (_clients.find(fd) != _clients.end())
				{
					existingClient(fd);
				}
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

void ServerLauncher::registerCGIFD(int cgiFD, int clientFD)
{
	_epoll.addFD(cgiFD, EPOLLIN);
	_cgiFDMap[cgiFD] = clientFD;
}

void ServerLauncher::removeCGIFD(int fd) {
	_cgiFDMap.erase(fd);
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
		setCloexecFlag(clientFd);
		_epoll.addFD(clientFd, EPOLLIN | EPOLLOUT);
		_clients[clientFd] = new Client(clientFd, server->getConfig(), _sessionManager, this);
	}
}

void ServerLauncher::existingClient(int clientFd)
{
	Client* client = _clients[clientFd];

	if (!client)
	return;

	HTTPRequest* http = NULL;
	try
	{
		http = client->readRequest();
		if (!http || http->method.empty() || http->uri.empty())
		{
			if (http)
				delete http;
			removeClient(clientFd);
			return;
		}
		Server* server = serverSelector(*http);
		if (server && server->getConfig().getServerName() != client->getConfigFileServer().getServerName())
		{
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
		if (http)
			delete http;
		removeClient(clientFd);
	}
}

Server* ServerLauncher::serverSelector(const HTTPRequest &http)
{
	std::string host = http.getHost();
	int port = http.getPort();

	if (host == "localhost")
		host = "127.0.0.1";

	Server* fallback = NULL;

	for (std::vector<Server*>::iterator it = _serverConfigOrder.begin(); it != _serverConfigOrder.end(); ++it)
	{
		const ConfigFileServer& config = (*it)->getConfig();
		const std::vector<std::pair<std::string, int> >& hostPorts = config.getHostPort();
		for (size_t i = 0; i < hostPorts.size(); ++i)
		{
			if (hostPorts[i].first == host && hostPorts[i].second == port)
				return (*it);
			if (!fallback && hostPorts[i].second == port)
				fallback = *it;
		}
	}
	return (fallback);
}

void ServerLauncher::removeClient(int clientFd)
{
	_epoll.removeFD(clientFd);
	if (_clients.find(clientFd) != _clients.end())
	{
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
