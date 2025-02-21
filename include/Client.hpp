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
#include "Utilities.hpp"

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
#include <stdio.h> //errno

class Client
{
	private:
		int			_clientSocket;
		std::string	_requestBuffer;
		std::string	_responseBuffer;
		ssize_t		_bytesSent;
		ServerConfig _currentConfig;

		void	handleGET(const std::string &path);
		void	handlePOST(const std::string &path, const std::string &body);
		void	handleDELETE(const std::string &path);

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
		void closeClient();
		void prepareResponse(const std::string &status, const std::string &contentType, const std::string &body);

		int getSocket() const;
};

#endif