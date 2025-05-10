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

class CGI;
struct CGIProcess
{
    int			socketPair[2];
    pid_t		pid;
	HTTPRequest	*http;
	CGI			*cgi;
};

class Server;
class ServerLauncher
{
	private:
		std::map<int, Server*>		_servers;
		std::map<int, Client*>		_clients;
		EPoll						_epoll;
		SessionManagement			_sessionManager;
		std::map<int, CGIProcess>	_cgiProcesses;

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
		EPoll	&getEpoll();
		void	addCGIProcess(int socketPair[2], pid_t pid, HTTPRequest *http, CGI *cgi);
};

#include "Server.hpp"
#include "CGI.hpp"

#endif
