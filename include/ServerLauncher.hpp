/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerLauncher.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 17:00:42 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/05/10 17:43:24 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERLAUNCHER_HPP
#define SERVERLAUNCHER_HPP

#include <vector>
#include <map>
#include <iostream>
#include <signal.h>
#include "Client.hpp"
#include "ConfigFile.hpp"
#include "EPoll.hpp"
#include "SessionManagement.hpp"

#define DEFAULT_CONFIG "default.conf"

class Server;
class ServerLauncher
{
	private:
		std::map<int, Server*>		_servers;
		std::vector<Server*>		_serverConfigOrder;
		std::map<int, Client*>		_clients;
		EPoll						_epoll;
		SessionManagement			_sessionManager;
		std::map<int, int>			_cgiFDMap;

		ServerLauncher &operator=(ServerLauncher const &rhs);
		ServerLauncher(ServerLauncher const &src);

		void	newClient(int serverFd);
		void	existingClient(int clientFd);
		void	removeClient(int clientFd);
		Server*	serverSelector(const HTTPRequest &http);
		
	public:
		ServerLauncher(void);
		~ServerLauncher(void);

		void	initServers(const std::string &configFile);
		void	loop();
		void	cleanupChild();
		void	registerCGIFD(int fd, int clientFd);
		EPoll	&getEpoll();
		void	removeCGIFD(int fd);
};

#include "Server.hpp"

#endif
