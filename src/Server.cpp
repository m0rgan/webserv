/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 12:02:13 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/15 12:02:13 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server() : _nfds(0)
{
	//getprotobyname
	_proto = getprotobyname("tcp");
	if (!_proto)
	{
		perror("getprotobyname");
		// return (1);
	}
}

Server::Server(const ServerConfig &config) : _nfds(0), currentConfig(config)
{
	_proto = getprotobyname("tcp");
	if (!_proto)
	{
		perror("getprotobyname");
	}
}

Server::Server(Server const &src) : currentConfig(src.currentConfig)  //must finish 
{
	this->_nfds = src._nfds;
	return;
}

Server &Server::operator=(Server const &rhs)
{
	if (this == &rhs)
		return (*this);
	this->_nfds = rhs._nfds;
	this->currentConfig = rhs.currentConfig;
	return (*this);
}

Server::~Server(void){}

int Server::sockets(const std::vector<int>& ports)
{
	int		serverSocket;
	size_t	j;

	for (j = 0; j < ports.size(); ++j)
	{
		serverSocket = createSocket();
		if (serverSocket < 0)
			return (1);
		if (configureSocket(serverSocket))
			return (1);
		if (bindAndListen(serverSocket, ports[j]))
			return (1);
		addToPoll(serverSocket);
	}
	return (0);
}

int Server::createSocket()
{
	int fd;

	fd = socket(AF_INET, SOCK_STREAM, _proto->p_proto);
	if (fd == -1)
	{
		perror("socket");
		return (-9); //handle error, -1 is socket error
	}
	return (fd);
}

//  Handling Errors and Disconnections
// if ((_pollFds[i].revents & POLLERR) || _pollFds[i].revents & POLLHUP)
//     _pruneSocket(sd, sS);
// If a socket has encountered an error (POLLERR) or has been closed by the client (POLLHUP), the function _pruneSocket(sd, sS) is called to handle cleanup.

int Server::configureSocket(int serverSocket)
{
	int	opt;

	opt = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		perror("setsockopt");
		close(serverSocket);
		return (1);
	}
	fcntl(serverSocket, F_SETFL, O_NONBLOCK);
	// int serverFlags = fcntl(serverSocket, F_GETFL, 0); // set socket non-blocking?
	// fcntl(serverSocket, F_SETFL, serverFlags | O_NONBLOCK);
	return (0);
}

int Server::bindAndListen(int serverSocket, int port)
{
	sockaddr_in serverAddr = {}; //set socket address and initialize struct to zeros
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	
	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		perror("bind");
		close(serverSocket);
		return (1);
	}
	if (listen(serverSocket, 10) == -1) // backlog should be 10 or? check
	{
		perror("listen");
		close(serverSocket);
		return (1);
	}
	std::cout << "Server listening on port: " << port << std::endl;
	return (0);
}

void Server::addToPoll(int serverSocket)
{
	pollfd serverPollfd = {serverSocket, POLLIN, 0};
	_fds.push_back(serverPollfd);
	_nfds++;
}


// while (!g_sigint) // to give program time to clean up
// {
// 	if ((poll(_pollFds.data(), _pollFds.size(), -1)) <= 0) continue ;
// The loop continues running until a global interrupt signal (g_sigint) is set.
void Server::loop()
{
	int	pollCount;

	std::cout << "Server setup complete. Waiting for connections..." << std::endl;
	signal(SIGURG, SignalHandler::handleUrgentData);
	for (;;)
	{
		pollCount = poll(_fds.data(), _nfds, -1);
		if (pollCount == -1)
		{
			perror("poll");
			break;
		}
		handleEvents();
	}
	cleanupSockets();
}

void Server::handleEvents()
{
	int	i;

	for (i = 0; i < _nfds; i++)
	{
		if (_fds[i].revents & POLLIN) //check if _fds[i] ready for reading
		{
			if (_fds[i].fd == _fds[0].fd)
				newClient();
			else // existing client socket I/O operations
				existingClient(i);
		}
	}
}

int Server::newClient()
{
	int			clientSocket;
	sockaddr_in	clientAddr;
	socklen_t	socketSize;

	socketSize = sizeof(clientAddr);
	clientSocket = accept(_fds[0].fd, (struct sockaddr*)&clientAddr, &socketSize); //creates CLIENTsocket
	if (clientSocket == -1)
	{
		perror("accept");
		return (1);
	}
	// int clientFlags = fcntl(clientSocket, F_GETFL, 0);
				// fcntl(clientSocket, F_SETFL, clientFlags | O_NONBLOCK);
	fcntl(clientSocket, F_SETFL, O_NONBLOCK);
	pollfd clientPollfd = {clientSocket, POLLIN | POLLOUT, 0}; //remove POLLOUT?
	_fds.push_back(clientPollfd);
	_nfds++;

	_clients[clientSocket] = new Client(clientSocket, currentConfig);

	std::cout << "Accepted new connection from client." << std::endl;
	return (0);
}

int Server::existingClient(int index)
{
	int clientSocket = _fds[index].fd;
	Client* client = _clients[clientSocket];

	try
	{
		client->readRequest();
		client->handleRequest();
		if (client->hasPendingData())
			client->writeResponse();
		// cookie and multiple cgi management.. do bonus or skip?
	}
	catch (const std::exception &e)
	{
		closeClient(index);
		return (1);
	}
	return (0);
}

void Server::closeClient(int index)
{
	int clientSocket = _fds[index].fd;

	delete _clients[clientSocket];
	_clients.erase(clientSocket);

	close(clientSocket);
	_fds[index] = _fds[_nfds - 1];
	_fds.pop_back();
	_nfds--;

	std::cout << "Closed connection with client." << std::endl;
}

void Server::cleanupSockets()
{
	int	i;

	for (i = 0; i < _nfds; i++)
		close(_fds[i].fd);
}


// Breakdown of Nginx’s Request Flow
// Here’s a simplified version of how Nginx handles a request:

// Client connects → A socket is accepted and an ngx_connection_t is created.
// Event Loop Detects POLLIN → Nginx reads the request but does not process it inside the read handler.
// Request Parsing → The HTTP request is parsed and stored in an ngx_http_request_t structure.
// Request Processing
// If it’s a static file request, it locates the file and prepares a response.
// If it’s a proxy request, it forwards the request to a backend.
// If it’s a FastCGI request, it communicates with PHP/CGI.
// The response is not sent immediately.
// Event Loop Detects POLLOUT → Only when the client is ready to receive data, Nginx sends the response.
// Response is Sent in Chunks → Nginx writes the response using a write event handler.

// How Should This Apply to Your Server?
// Your current model can be improved to be more like Nginx:

// ✅ Prepare Response First → Your Client::handleRequest() should prepare the response but not send it immediately.
// ✅ Write Response in a Separate Step → A Client::writeResponse() function should be triggered when POLLOUT is detected.
// ✅ Use a State Machine for Clients → Track if the client is waiting for a response or ready to send data.