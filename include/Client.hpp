/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 16:55:28 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/15 17:46:22 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

#include <HttpParser.hpp>
#include <ServerConfig.hpp>

#include <sstream>
#include <fstream>
#include <istream>
#include <ostream>
#include <map>
#include <poll.h>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

class Client
{
	private:
		int			_clientSocket;
		std::string	_requestBuffer;
		std::string	_responseBuffer;
		size_t		_bytesSent;
		ServerConfig _currentConfig;

		// void	handleRequest(int clientSocket, const std::string &requestHTTP);
		void	handleGET(const std::string &path);
		void	handlePOST(const std::string &path, const std::string &body);
		void	handleDELETE(const std::string &path);
		// void	prepareResponse(int clientSocket, const std::string &status, const std::string &contentType, const std::string &body);

	public:
		Client(void);
		Client(int socket, const ServerConfig& config);
		Client(Client const &src);
		Client &operator=(Client const &rhs);
		~Client(void);

		void readRequest();
		void handleRequest();
		bool hasPendingData() const;
		void writeResponse();

		void processRequest();
		void sendResponse();
		// int getRequest();
		void writeClient();
		void closeClient();
		void prepareResponse(const std::string &status, const std::string &contentType, const std::string &body);
		void prepareBinaryResponse(const std::string &status, const std::string &contentType, const std::vector<char> &body);

		std::string getMimeType(const std::string &extension) const; // maybe move?

		int getSocket() const;
};

#endif