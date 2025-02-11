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

#include <sys/socket.h> //socket
#include <netdb.h> //getprotobyname
#include <fcntl.h> //fcntl
#include <arpa/inet.h> //htons host to network short
#include <unistd.h> //close
#include <poll.h> //pollfd

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		//err msg
		return (1);
	}
	ConfigParser configFile(argv[1]); // Automatically parses on creation
	//exceptions from configFile and exit??

	const ServerConfig &firstServer = configFile.getServers()[0];
	const std::vector<int> &ports = firstServer.getPorts();

std::cout << "Parsed configuration file successfully." << std::endl;
std::cout << "Server name: " << firstServer.getServerName() << std::endl;
for (size_t i = 0; i < ports.size(); ++i)
	std::cout << "Listening on port: " << ports[i] << std::endl;

	//getprotobyname
	struct protoent *proto = getprotobyname("tcp");
	if (!proto)
	{
		perror("getprotobyname");
		return (1);
	}

	//create sockets
	struct pollfd fds[100]; //is there a maximum amount of sockets?
	int nfds = 0;

	for (size_t j = 0; j < ports.size(); ++j)
	{
		int serverSocket = socket(AF_INET, SOCK_STREAM, proto->p_proto);
		if (serverSocket == -1)
		{
			perror("socket");
			return (1);
		}
		/* Sockets of type SOCK_STREAM are full-duplex byte streams, similar to pipes.  A stream socket must be in a connected state before any data may
be sent or received on it.  A connection to another socket is created with a connect(2) or connectx(2) call.  Once connected, data may be
transferred using read(2) and write(2) calls or some variant of the send(2) and recv(2) calls.  When a session has been completed a close(2)
may be performed.  Out-of-band data may also be transmitted as described in send(2) and received as described in recv(2).

The communications protocols used to implement a SOCK_STREAM insure that data is not lost or duplicated.  If a piece of data for which the peer
protocol has buffer space cannot be successfully transmitted within a reasonable length of time, then the connection is considered broken and
calls will indicate an error with -1 returns and with ETIMEDOUT as the specific code in the global variable errno.  The protocols optionally
keep sockets “warm” by forcing transmissions roughly every minute in the absence of other activity.  An error is then indicated if no response
can be elicited on an otherwise idle connection for a extended period (e.g. 5 minutes).  A SIGPIPE signal is raised if a process sends on a
broken stream; this causes naive processes, which do not handle the signal, to exit. 

An fcntl(2) call can be used to specify a process group to receive a SIGURG signal when the out-of-band data arrives.  It may also enable non-
blocking I/O and asynchronous notification of I/O events via SIGIO.

The operation of sockets is controlled by socket level options.  These options are defined in the file ⟨sys/socket.h⟩.  Setsockopt(2) and
getsockopt(2) are used to set and get options, respectively.*/

		int flags = fcntl(serverSocket, F_GETFL, 0); // set socket non-blocking
		fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);

		struct sockaddr_in serverAddr; //set socket address
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = htons(ports[j]); // Use port from getPorts
		serverAddr.sin_addr.s_addr = INADDR_ANY;
		memset(&(serverAddr.sin_zero), '\0', 8);

		// bind socket --to ip address and port
		if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1)
		{
			perror("bind");
			close(serverSocket); // bind error -> closesocket return 1
			return (1);
		}

		// listen to socket --await for connections
		if (listen(serverSocket, 10) == -1) // backlog should be 10 or? check
		{
			perror("listen");
			close(serverSocket); // listen error -> closesocket return 1
			return (1);
		}

std::cout << "Server socket created, bound, and listening on port: " << ports[j] << std::endl;

		fds[nfds].fd = serverSocket;
		fds[nfds].events = POLLIN;
		nfds++;
	}
std::cout << "Server setup complete. Waiting for connections..." << std::endl;

	//accept (HERE IS THE BLOCKING) -shouldnt matter because socket set to NB
	for (;;)
	{
		int pollCount = poll(fds, nfds, -1);
		if (pollCount == -1)
		{
			perror("poll");
			break;
		}
		for (int i = 0; i < nfds; i++)
		{
			if (fds[i].revents & POLLIN)
			{
				if (fds[i].fd == fds[0].fd) // there is a new client socket
				{
					struct sockaddr_in clientAddr;
					socklen_t sinSize = sizeof(struct sockaddr_in);
					int clientSocket = accept(fds[0].fd, (struct sockaddr *)&clientAddr, &sinSize);
					if (clientSocket == -1)
					{
						perror("accept");
						break;
					}
					int flags = fcntl(clientSocket, F_GETFL, 0);
					fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);

					fds[nfds].fd = clientSocket;
					fds[nfds].events = POLLIN | POLLOUT;
					nfds++;
std::cout << "Accepted new connection from client." << std::endl;
				}
				else // existing client socket I/O operations
				{
					char buffer[1024]; // make dynamic/must change
					int bytesRead = read(fds[i].fd, buffer, sizeof(buffer) - 1);
					if (bytesRead > 0)
					{
						buffer[bytesRead] = '\0';
						std::string request(buffer);
std::cout << "Received request: " << request << std::endl;
						firstServer.handleRequest(fds[i].fd, request);
						// cookie and multiple cgi management.. do bonus or skip?
					}
					else
					{
						close(fds[i].fd); // close client socket
						fds[i] = fds[nfds - 1]; // remove cs from poll array
						nfds--;
std::cout << "Closed connection with client." << std::endl;
					}
				}
			}
		}
	}
	for (int i = 0; i < nfds; i++)
		close(fds[i].fd);  //closesocket
	return (0);
}

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