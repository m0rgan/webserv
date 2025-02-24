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


struct ClientState
{
    std::string response;
    size_t bytesSent;
};
class Server
{
	private:
		std::string				_name;
		std::vector<int>		_ports;
		std::vector<pollfd> 	_fds;
		int						_nfds;
		struct protoent			*_proto;
		ServerConfig			_currentConfig;
		std::map<int, Client*>	_clients;

		int		createSocket();
		int		configureSocket(int serverSocket);
		int		bindAndListen(int serverSocket, int port);
		void	addToPollList(int serverSocket);

		int		newClient(int index);
		int		existingClient(int index);
		void	closeClient(int index);
		void	cleanupSockets();

		
	public:
		Server(void);
		Server(Server const &src);
		Server &operator=(Server const &rhs);
		~Server(void);

		Server(const ServerConfig &config);
		const ServerConfig& getConfig() const;
		const std::string getName() const;
		const std::vector<int> getPorts() const;
		void	handleEvent(const pollfd &pfd);
		int		sockets();
		const std::vector<pollfd> &getSockets() const;
};

#endif