/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 11:08:29 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/23 13:52:59 by gabrielfern      ###   ########.fr       */
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

struct HttpRequest
{
	std::string method;
	std::string uri;
	std::string httpVersion;
	std::map<std::string, std::string> headers;
	std::string body;
	size_t	contentLength;
};

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
		
		HttpRequest	request;
		void parserHeaders(const std::string &rawRequest);
		void parserBody(const std::string &rawRequest);
		std::string resolveFilePath(const ConfigFileServer &config) const;
		size_t parseContentLength(std::string contentLengthStr);
		void logRequest(const std::string timestamp) const;
		const std::string& getHost() const;
		int getPort() const;
};

//include httprequest struct in class
//include response status code? to save last code of request?
//make responses like a template in this class instead of inside member function in Client?
// HTTP response status codes

// HTTP response status codes indicate whether a specific HTTP request has been successfully completed. Responses are grouped in five classes:
// Informational responses (100 – 199)
// Successful responses (200 – 299)
// Redirection messages (300 – 399)
// Client error responses (400 – 499)
// Server error responses (500 – 599)
#endif