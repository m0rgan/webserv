/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 14:58:18 by migumore          #+#    #+#             */
/*   Updated: 2025/02/23 11:22:09 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <HTTPRequest.hpp>

HTTPRequest::HTTPRequest(void){};

HTTPRequest::HTTPRequest(HTTPRequest const &src)
{
	(void)src;
};

HTTPRequest HTTPRequest::operator=(HTTPRequest const &rhs)
{
	(void)rhs;
	return (*this);
};

HTTPRequest::~HTTPRequest(void){};

void HTTPRequest::parser(const std::string &rawRequest)
{
	std::istringstream	requestStream(rawRequest);
	std::string			line;

	if (std::getline(requestStream, line))
	{
		std::istringstream lineStream(line);
		lineStream >> request.method >> request.uri >> request.httpVersion;
	}
	while (std::getline(requestStream, line) && line != "\r")
	{
		std::size_t colonPos = line.find(':');
		if (colonPos != std::string::npos)
		{
			std::string key = line.substr(0, colonPos);
			std::string value = line.substr(colonPos + 1);
			while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
				value.erase(0, 1);
			request.headers[key] = value;
		}
	}
	if (request.headers.find("Content-Length") != request.headers.end())
	{
		std::string body;
		std::getline(requestStream, body, '\0');
		request.body = body;
	}
}