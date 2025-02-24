/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerLauncher.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 17:01:19 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/21 17:01:19 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerLauncher.hpp"

static ServerLauncher* serverLauncherInstance = nullptr;

void handleSIGINT(int sig)
{
	std::cerr << " -Caught signal " << sig << " - terminate ./webserv" << std::endl;
	if (serverLauncherInstance)
		serverLauncherInstance->stopServers();
	kill(0, SIGTERM); // kill main process?
}

ServerLauncher::ServerLauncher(void) {}; //fix

ServerLauncher::ServerLauncher(const std::string &configFile)
{
	serverLauncherInstance = this;
	signal(SIGPIPE, SIG_IGN); //can be handled on recv or send?
	signal(SIGINT, handleSIGINT); //make one macro signal handler?

	ConfigParser parsedConfigFile(configFile);
	parsedConfigFile.printConfig(); //delete its debug
	_serverBlocks = parsedConfigFile.getServers();

	size_t	i;
	for (i = 0; i < _serverBlocks.size(); ++i)
	{
		std::cout << "Launching server: " << _serverBlocks[i].getName() << std::endl;
		Server* server = new Server(_serverBlocks[i]);
		_servers.push_back(server);
		if (_servers.back()->sockets())
		{
			delete _servers.back();
			_servers.pop_back(); //handle bad server block and launch rest or? how does nginx do?
			throw std::runtime_error("server");
		}
		const std::vector<pollfd> &sockets = _servers.back()->getSockets();
		for (size_t j = 0; j < sockets.size(); ++j)
			_socketServer[sockets[j].fd] = _servers.back();
	}
}

ServerLauncher::ServerLauncher(const ServerLauncher &src)
{
	for (size_t i = 0; i < src._servers.size(); ++i)
		_servers.push_back(new Server(*src._servers[i]));
	_serverBlocks = src._serverBlocks;
	_socketServer = src._socketServer;
}

ServerLauncher &ServerLauncher::operator=(const ServerLauncher &rhs)
{
	if (this != &rhs)
	{
		for (size_t i = 0; i < _servers.size(); ++i)
			delete _servers[i];
		_servers.clear();

		for (size_t i = 0; i < rhs._servers.size(); ++i)
			_servers.push_back(new Server(*rhs._servers[i]));
		_serverBlocks = rhs._serverBlocks;
		_socketServer = rhs._socketServer;
	}
	return (*this);
}

ServerLauncher::~ServerLauncher(void)
{
	for (size_t i = 0; i < _servers.size(); ++i)
		delete _servers[i];
	_servers.clear();
}

void ServerLauncher::loop()
{
	int	readyEvents;

	signal(SIGURG, SignalHandler::handleUrgentData);
	for (;;)
	{
		socketsList();
		readyEvents = poll(_pollfds.data(), _pollfds.size(), 0); // or TIMEOUT?
		if (readyEvents == -1)
		{
			std::cerr << "poll: " << strerror(errno) << std::endl;
			break;
		}
		handleEvents();
	}
	cleanupSockets();
}

void ServerLauncher::socketsList()
{
	size_t				i;
	size_t				j;
	std::vector<pollfd> socketsList;

	_pollfds.clear();
	_socketServer.clear();	
	for (i = 0; i < _servers.size(); ++i)
	{
		socketsList = _servers[i]->getSockets();
		for (j = 0; j < socketsList.size(); ++j)
		{
			pollfd pfd = socketsList[j];
			_pollfds.push_back(pfd);
			_socketServer[pfd.fd] = _servers[i];
		}
	}
}

void ServerLauncher::handleEvents()
{
	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_pollfds[i].revents & POLLIN)
		{
			Server *server = _socketServer[_pollfds[i].fd];
			server->handleEvent(_pollfds[i]);
		}
	}
}

void ServerLauncher::cleanupSockets()
{
	for (std::map<int, Server*>::iterator it = _socketServer.begin(); it != _socketServer.end(); ++it)
		close(it->first);
}

void ServerLauncher::stopServers()
{
	for (size_t i = 0; i < _servers.size(); ++i)
		delete _servers[i];
	_servers.clear();
}

// void ServerLauncher::restartServer(size_t index)
// {
// 	if (index >= _servers.size())
// 	{
// 		std::cerr << "Invalid server index: " << index << std::endl;
// 		return;
// 	}
// 	delete _servers[index];
// 	try
// 	{
// 		_servers[index] = new Server(_servers[index]->getConfig());
// 		if (_servers[index]->sockets(_servers[index]->getConfig().getPorts()))
// 		{
// 			throw std::runtime_error("Failed to restart server");
// 		}
// 	}
// 	catch (const std::exception &e)
// 	{
// 		std::cerr << "Error restarting server: " << e.what() << std::endl;
// 		_servers[index] = nullptr; // Mark as inactive
// 	}
// }
