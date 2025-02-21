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

static std::vector<pid_t> serverPIDs;

void handleSigint(int sig)
{
	std::cerr << "Caught signal " << sig << ", terminating servers..." << std::endl;
	for (size_t i = 0; i < serverPIDs.size(); ++i)
		kill(serverPIDs[i], SIGTERM);
	// kill main process?
}

ServerLauncher::ServerLauncher(const std::string &configFile)
{
	signal(SIGPIPE, SIG_IGN); //can be handled on recv or send?
	signal(SIGINT, handleSigint); //make one macro signal handler?

	ConfigParser parsedConfigFile(configFile);
	const std::vector<ServerConfig> &serverBlocks = parsedConfigFile.getServers();

	size_t	i;
	Server	*server;

	for (i = 0; i < serverBlocks.size(); ++i)
	{
		std::cout << "Launching server: " << serverBlocks[i].getName() << std::endl;
		server = new Server(serverBlocks[i]);

		if (server->sockets(serverBlocks[i].getPorts()))
		{
			delete server;
			throw std::runtime_error("sockets");
		}
		_servers.push_back(server);
	}
}

ServerLauncher::ServerLauncher(const ServerLauncher &src)
{
	for (size_t i = 0; i < src._servers.size(); ++i)
		_servers.push_back(new Server(*src._servers[i]));
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
	}
	return (*(this));
}

ServerLauncher::~ServerLauncher()
{
	for (size_t i = 0; i < _servers.size(); ++i)
		delete _servers[i];
	_servers.clear();
}

void ServerLauncher::launch()
{
	size_t	i;
	for (i = 0; i < _servers.size(); ++i)
	{
		pid_t pid = fork();
		if (pid == -1)
		{
			std::cerr << "Error: Failed to fork process for server " << i << std::endl;
			continue;
		}
		if (pid == 0)
		{
			_servers[i]->loop();
			kill(getpid(), SIGTERM);
		}
		else
			serverPIDs.push_back(pid);
	}
	for (i = 0; i < serverPIDs.size(); ++i)
	{
		int status;
		waitpid(serverPIDs[i], &status, 0);
	}
}

// void ServerLauncher::stopServers()
// {
// 	for (size_t i = 0; i < _servers.size(); ++i)
// 	{
// 		// _servers[i]->shutdownServer();
// 		delete _servers[i];
// 	}
// 	_servers.clear();
// }

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
