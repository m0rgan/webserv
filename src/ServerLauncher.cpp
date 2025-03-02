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
		signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE to prevent crashes on broken pipes
		signal(SIGINT, handleSIGINT);

		initServers(configFile);
		loop();
	}

	void ServerLauncher::initServers(const std::string &configFile)
	{
		ConfigFile parsedConfigFile(configFile);
		//try catch for configfile?
		std::vector<ServerConfig> configs = parsedConfigFile.getServers();
		std::vector<Server*> buffer;
		for (size_t i = 0; i < configs.size(); ++i)
		{
			try
			{
				std::cout << "[INFO] Launching server: " << configs[i].getName() << std::endl;
				buffer.push_back(new Server(configs[i]));
				//handle new error?
				if (buffer.back()->sockets())
				{
					delete buffer.back();
					buffer.pop_back();
					throw std::runtime_error("Server socket setup failed.");
				}

				const std::vector<pollfd> &serverSockets = buffer.back()->getSockets();
				for (size_t j = 0; j < serverSockets.size(); ++j)
				{
					if (_servers.find(serverSockets[j].fd) != _servers.end())
					{
						std::cerr << "[ERROR] Failed to bind and listen on " << serverSockets[j].fd << std::endl;
						delete buffer.back();
						buffer.pop_back();
						throw std::runtime_error("Server socket setup failed.");
					}
					_pollfds.push_back(serverSockets[j]);
					_servers[serverSockets[j].fd] = buffer.back();
				}
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
			int readyEvents = poll(_pollfds.data(), _pollfds.size(), POLL_TIMEOUT);
			if (readyEvents == -1)
			{
				std::cerr << "[ERROR] poll() failed: " << strerror(errno) << std::endl;
				break;
			}
			else if (readyEvents == 0)
				continue;
			dispatchEvents();
		}
		cleanupSockets();
	}

	void ServerLauncher::dispatchEvents()
	{
		for (size_t i = 0; i < _pollfds.size(); ++i)
		{
			int fd = _pollfds[i].fd;
			if (_pollfds[i].revents & POLLIN)
			{
				if (_servers.find(fd) != _servers.end())
					newClient(fd);
				else
					existingClient(fd);
				break;
			}
			if (_pollfds[i].revents & POLLOUT)
			{
				if (_clients.find(fd) != _clients.end())
					if (_clients[fd]->hasPendingData())
						_clients[fd]->writeResponse();
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
			pollfd clientPollfd = {clientFd, POLLIN | POLLOUT, 0};
			_pollfds.push_back(clientPollfd);
			_clients[clientFd] = new Client(clientFd, server->getConfig());
		}
	}

	void ServerLauncher::existingClient(int clientFd)
	{
		Client* client = _clients[clientFd];

		if (!client)
		{
			std::cerr << "[ERROR] No client found for FD: " << clientFd << std::endl;
			return;
		}
		try
		{
			client->readRequest();
			client->handleRequest();
			// cookie and multiple cgi management.. do bonus or skip?
		}
		catch (const std::exception &e)
		{
			std::cerr << "[ERROR] Client error: " << e.what() << std::endl;
			closeClient(clientFd);
		}
	}

	void ServerLauncher::closeClient(int clientFd)
	{
		close(clientFd);
		for (size_t i = 0; i < _pollfds.size(); ++i)
		{
			if (_pollfds[i].fd == clientFd)
			{
				_pollfds.erase(_pollfds.begin() + i);
				break;
			}
		}
		if (_clients.find(clientFd) != _clients.end())
		{
			delete _clients[clientFd];
			_clients.erase(clientFd);
		}
	}

	void ServerLauncher::cleanupSockets()
	{
		for (std::map<int, Server*>::iterator it = _servers.begin(); it != _servers.end(); ++it)
			close(it->first);
	}

	void ServerLauncher::stopServers()
	{
		for (std::map<int, Server*>::iterator it = _servers.begin(); it != _servers.end(); ++it)
			delete it->second;
		_servers.clear();
	}