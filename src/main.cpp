/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 15:02:32 by migumore          #+#    #+#             */
/*   Updated: 2025/02/05 18:13:18 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <HttpParser.hpp>
#include <ConfigParser.hpp>
#include <iostream>

int main(int argc, char *argv[])
{
	if (argc == 2)
	{
		ConfigParser parser(argv[1]); // Automatically parses on creation
		const std::vector<ServerConfig> &servers = parser.getServers();

		for (size_t i = 0; i < servers.size(); i++)
		{
			std::cout << "=== Server Configuration " << i + 1 << " ===" << std::endl;
			servers[i].printConfig();
			std::cout << std::endl;
		}
	}
	// std::string rawRequest =
	//     "POST /upload HTTP/1.1\r\n"
	//     "Host: example.com\r\n"
	//     "User-Agent: curl/7.68.0\r\n"
	//     "Content-Length: 11\r\n"
	//     "Content-Type: text/plain\r\n"
	//     "\r\n"
	//     "Hello World";

	// HttpParser parser;
	// HttpRequest request = parser.parseRequest(rawRequest);

	// std::cout << "=== Parsed HTTP Request ===" << std::endl;
	// request.printRequest();

	return 0;
}
