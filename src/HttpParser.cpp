/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 14:58:18 by migumore          #+#    #+#             */
/*   Updated: 2025/02/05 18:01:02 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <HttpParser.hpp>

HttpRequest HttpParser::parseRequest(const std::string &rawRequest)
{
	HttpRequest request;
	std::istringstream requestStream(rawRequest);
	std::string line;

	// 1️⃣ Parse the request line: METHOD URI HTTP/VERSION
	if (std::getline(requestStream, line))
	{
		std::istringstream lineStream(line);
		lineStream >> request.method >> request.uri >> request.httpVersion;
	}

	// 2️⃣ Parse headers
	while (std::getline(requestStream, line) && line != "\r")
	{
		std::size_t colonPos = line.find(':');
		if (colonPos != std::string::npos)
		{
			std::string key = line.substr(0, colonPos);
			std::string value = line.substr(colonPos + 1);
			while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
				value.erase(0, 1); // Trim leading spaces
			request.headers[key] = value;
		}
	}

	// 3️⃣ Parse the body (if Content-Length is provided)
	if (request.headers.find("Content-Length") != request.headers.end())
	{
		std::string body;
		std::getline(requestStream, body, '\0'); // Read remaining data
		request.body = body;
	}

	return request;
}