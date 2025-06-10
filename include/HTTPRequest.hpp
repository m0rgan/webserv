/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 11:08:29 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/04/01 14:39:35 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <iostream>
#include <sys/stat.h>
#include <Utilities.hpp>
#include <ConfigFileServer.hpp>
#include "ErrorPage.hpp"

class Client;

class HTTPRequest
{
	private:
		std::string	_host;
		int			_port;
		

	public:
		HTTPRequest(void);
		HTTPRequest(HTTPRequest const &src);
		HTTPRequest operator=(HTTPRequest const &rhs);
		~HTTPRequest(void);
		
		std::string method;
		std::string uri;
		std::string httpVersion;
		std::map<std::string, std::string> headers;
		std::string body;
		size_t	contentLength;
		std::string	resolvedFilePath;

		std::string path;
		std::string query;
		std::string fragment;
	
		void parserHeaders(const std::string &rawRequest);
		std::string resolveFilePath(const ConfigFileServer &config) const;
		size_t parseContentLength(std::string contentLengthStr);
		void logRequest(const std::string timestamp) const;
		const std::string& getHost() const;
		int getPort() const;

		bool isMethodAllowed(const ConfigFileServerLocation *location) const;
		bool handleReturnDirective(int statusCode, const std::string &redirectUrl, Client &client) const;
		bool serverReturn(const ConfigFileServer &config, Client &client) const;
		bool locationReturn(const ConfigFileServerLocation *location, Client &client) const;
		const ConfigFileServerLocation* matchLocation(const ConfigFileServer &config) const;
		bool validateRequest(const ConfigFileServer &config, Client &client);
		void parseURI();
};

#include "Client.hpp"

#endif