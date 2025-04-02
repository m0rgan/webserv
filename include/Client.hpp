/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 16:55:28 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/04/01 17:27:56 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

#include <HTTPRequest.hpp>
#include <ConfigFileServer.hpp>
#include <CGI.hpp>
#include "Utilities.hpp"
#include <HTTPResponse.hpp>
#include <ErrorPage.hpp>
#include <SessionManagement.hpp>
#include <Cookies.hpp>
#include <ServerLauncher.hpp>

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
		int					_clientSocket;
		std::string			_requestBuffer;
		std::string			_responseBuffer;
		ssize_t				_bytesSent;
		ConfigFileServer	_currentConfig;
		bool				_keepAlive;

		void	handleGET(HTTPRequest *http);
		void	handlePOST(HTTPRequest *http);
		void	handleDELETE(HTTPRequest *http);
		bool	lengthData(HTTPRequest &http);
		bool	chunkedData(HTTPRequest &http);
		bool	routeToCGI(std::string requestURI);
		
		SessionManagement	&_sessionManager;
		Cookies				_cookies;
		void				handleCookies(HTTPRequest &http);
		
	public:
		Client(void);
		Client(Client const &src);
		Client &operator=(Client const &rhs);
		~Client(void);
		
		Client(int socket, const ConfigFileServer &config, SessionManagement &sessionManager);
		HTTPRequest readRequest();
		void handleRequest(HTTPRequest &http, ServerLauncher* server);
		bool hasPendingData() const;
		void writeResponse();
		
		bool keepAlive() const;
		
		const ConfigFileServer& getConfigFileServer() const;
		void setConfigFileServer(const ConfigFileServer &config);
		void resetState(void);
		
		void	prepareResponse(int statusCode, const std::string &contentType, const std::string &body, const std::string &redirectUrl, const std::string &additionalHeaders);
		void	prepareErrorResponse(int statusCode);
	};

#endif