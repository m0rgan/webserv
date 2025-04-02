/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerLauncher.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 17:00:42 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/31 19:36:27 by migumore         ###   ########.fr       */
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
		std::map<int, Server*>	_servers;
		std::map<int, Client*>	_clients;
		EPoll					_epoll;
		SessionManagement		_sessionManager;

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
};

#include "Server.hpp"

#endif
