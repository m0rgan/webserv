/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 11:57:53 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/26 16:51:54 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>

#include <SignalHandler.hpp>
#include <ConfigFileServer.hpp>

#include <Client.hpp>
#include <EPoll.hpp>
#include <netdb.h> //getprotobyname
#include <fcntl.h> //fcntl
#include <poll.h> //pollfd
#include <sstream> //ostringstream
#include <fstream> //istringstream
#include <map> //map

class Server
{
	private:
		std::vector<std::pair <std::string, int> > 	_hostPort; //hay que confirmar que el numero del puerto es valido
		std::vector<int>	 						_fds;
		struct protoent								*_proto;
		ConfigFileServer							_currentConfig;
		

		int		createSocket(int family);
		void	configureSocket(int serverSocket);
		int		getAddressProtocol(const std::string &host);
		void	bindAndListen(int serverSocket, const std::string &host, int port);
		void	addToFDList(int serverSocket);
		
		public:
			Server(void);
			Server(const ConfigFileServer &config);
			Server(Server const &src);
			Server &operator=(Server const &rhs);
			~Server(void);
			
			int							acceptClient(int index);
			const ConfigFileServer&		getConfig() const;
			void						sockets();
			const std::vector<int>		&getSockets() const;
			void 						addSocketsToEpoll(EPoll &epoll);
};

#endif