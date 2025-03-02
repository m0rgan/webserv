/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 11:57:53 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/15 11:57:53 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>

#include <SignalHandler.hpp>
#include <ServerConfig.hpp>

#include <Client.hpp>
#include <netdb.h> //getprotobyname
#include <fcntl.h> //fcntl
#include <poll.h> //pollfd
#include <sstream> //ostringstream
#include <fstream> //istringstream
#include <map> //map


class Server
{
	private:
		std::string									_name;
		std::vector<int>							_ports;
		std::vector<std::string>					_hosts;
		std::vector<std::pair <std::string, int> > 	_hostPort;
		std::vector<pollfd> 						_fds;
		int											_nfds;
		struct protoent								*_proto;
		ServerConfig								_currentConfig;
		

		int		createSocket(int family);
		int		configureSocket(int serverSocket);
		int		getAddressProtocol(const std::string &host);
		int		bindAndListen(int serverSocket, const std::string &host, int port);
		void	addToPollList(int serverSocket);
		
		public:
		Server(void);
		Server(Server const &src);
		Server &operator=(Server const &rhs);
		~Server(void);
		
		Server(const ServerConfig &config);
		int		acceptClient(int index);
		const ServerConfig& getConfig() const;
		const std::string getName() const;
		const std::vector<int> getPorts() const;
		const std::vector<std::string> getHosts() const;
		int		sockets();
		const std::vector<pollfd> &getSockets() const;
};

#endif