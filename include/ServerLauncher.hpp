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

#include "Server.hpp"
#include "ConfigFile.hpp"
#include <vector>
#include <iostream>
#include <csignal>
#include <map>
#include <list>
#include <unistd.h>

#define POLL_TIMEOUT 5000

class ServerLauncher
{
	private:
		std::map<int, Server*>	_servers;
		std::map<int, Client*>	_clients;
		std::vector<pollfd>		_pollfds;
		
		void initServers(const std::string &configFile);
		void dispatchEvents();
		void newClient(int serverFd);
		void existingClient(int clientFd);
		void closeClient(int clientFd);
		void cleanupSockets();
		void loop();
		
	public:
		ServerLauncher(void);
		ServerLauncher(const std::string &configFile);
		ServerLauncher(ServerLauncher const &src);
		ServerLauncher &operator=(ServerLauncher const &rhs);
		~ServerLauncher(void);
		void stopServers();

};

#endif
