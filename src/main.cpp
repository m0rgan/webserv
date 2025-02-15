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

#include <ConfigParser.hpp>
#include <iostream>

#include <Server.hpp>

#include <sys/socket.h> //socket
#include <arpa/inet.h> //htons host to network short
#include <unistd.h> //close
#include <poll.h> //pollfd

int main(int argc, char *argv[])
{
	if (argc != 2)
		return (std::cerr << "How to use: " << argv[0] << " <config_file>" << std::endl, (1));
	//is the logic meant to be if config file fails then use default config or if no config file is set?

	signal(SIGPIPE, SIG_IGN); //handle clients disconnecting unexpectedly

	ConfigParser configFile(argv[1]); // Automatically parses on creation
	//exceptions from configFile and exit??

	const ServerConfig &firstServer = configFile.getServers()[0];

	std::cout << "Parsed configuration file successfully." << std::endl;
	std::cout << "Server name: " << firstServer.getName() << std::endl;

	Server webserv;
	webserv.sockets(firstServer.getPorts());
	webserv.loop();
	
	return (0);
}

//GET
//curl -v http://localhost:8080/
//curl -v http://localhost:9090/api
//POST
//curl -v -X POST -d "name=John&age=30" http://localhost:8080/submit
//curl -v -X POST -d "name=John&age=30" http://localhost:9090/api/submit
//DELETE
//curl -v -X DELETE http://localhost:8080/file-to-delete

	// const std::vector<int> &ports = firstServer.getPorts();

	// for (size_t j = 0; j < ports.size(); ++j)

	// const std::vector<ServerConfig> &servers = parser.getServers();
	// 	for (size_t i = 0; i < servers.size(); i++)
	// 	{
	// 		std::cout << "=== Server Configuration " << i + 1 << " ===" << std::endl;
	// 		servers[i].printConfig();
	// 		std::cout << std::endl;
	// 	}
	//setCookie(clientSocket, "sessionId", "123456");
	//std::string getCookie(const std::string &request, const std::string &cookieName);
	//handleCGI(clientSocket, "/path/to/cgi/script");

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