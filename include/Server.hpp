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
#include <HttpParser.hpp>
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
		std::vector<pollfd>		_fds; //does vector include all possible fds ? is there a maximum?
		int						_nfds;
		struct protoent			*_proto;
		ServerConfig			currentConfig;
		std::map<int, ClientState> clientStates; // Map of client socket to ClientState
		std::map<int, Client*>	_clients;

		int		createSocket();
		int		configureSocket(int serverSocket);
		int		bindAndListen(int serverSocket, int port);
		void	addToPoll(int serverSocket);
		void	handleEvents();
		int		newClient();
		int		existingClient(int index);
		int		readRequest(int index, std::string &request);
		void	closeClient(int index);
		void	cleanupSockets();
		void	handleRequest(int clientSocket, const std::string &requestHTTP);
		void	handleGET(int clientSocket, const std::string &path);
		void	handlePOST(int clientSocket, const std::string &path, const std::string &body);
		void	handleDELETE(int clientSocket, const std::string &path);
		void	sendResponse(int clientSocket, const std::string &status, const std::string &contentType, const std::string &body);

		void writeClient(int index);
		std::string getMimeType(const std::string &extension) const; // maybe move?
	public:
		Server(void);
		Server(Server const &src);
		Server &operator=(Server const &rhs);
		~Server(void);

		Server(const ServerConfig &config);
		int		sockets(const std::vector<int> &ports);
		void	loop();
};

#endif