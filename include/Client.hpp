/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 16:55:28 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/23 15:37:59 by gabrielfern      ###   ########.fr       */
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
		void	closeClient();
		bool	lengthData(HTTPRequest &http);
		bool	chunkedData(HTTPRequest &http);
		const ConfigFileServerLocation *matchLocation(const HTTPRequest &http) const;
		bool	serverReturn(void);
		bool	locationReturn(const ConfigFileServerLocation *location);
		bool	handleReturnDirective(int statusCode, const std::string &redirectUrl);
		void	prepareResponse(int statusCode, const std::string &contentType, const std::string &body, const std::string &redirectUrl, const std::string &additionalHeaders);
		void	prepareErrorResponse(int statusCode, const std::string &contentType, const std::string &body);
		void	serveErrorResponse(int statusCode);
		bool	routeToCGI(std::string requestURI);
		bool	validateRequest(HTTPRequest &http);

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
		void handleRequest(HTTPRequest &http);
		bool hasPendingData() const;
		void writeResponse();
		
		int getSocket() const;
		bool keepAlive() const;
		
		bool isMethodAllowed(const ConfigFileServerLocation *location, const std::string &method);
		
		const ConfigFileServer& getConfigFileServer() const;
		void setConfigFileServer(const ConfigFileServer &config);
		void resetState(void);
	};

#endif