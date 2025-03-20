/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerLauncher.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 17:00:42 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/21 17:00:42 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERLAUNCHER_HPP
#define SERVERLAUNCHER_HPP

#include <vector>
#include <map>
#include <iostream>
#include <signal.h>
#include "Server.hpp"
#include "Client.hpp"
#include "ConfigFile.hpp"
#include "EPoll.hpp"

#define DEFAULT_CONFIG "default.conf"

class ServerLauncher
{
	private:
		std::map<int, Server*>	_servers;
		std::map<int, Client*>	_clients;
		EPoll					_epoll;
		SessionManagement		_sessionManager;

		void newClient(int serverFd);
		void existingClient(int clientFd);
		void closeClient(int clientFd);
		Server* serverSelector(const HTTPRequest &http);

	public:
		ServerLauncher(void);
		ServerLauncher(const std::string &configFile);
		~ServerLauncher(void);
		void initServers(const std::string &configFile);
		void loop();
		void stopServers();
};

#endif
