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

Server::Server(Server const &src) : currentConfig(src.currentConfig)
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
		if (pollCount == -1) {
			perror("poll");
			break;
		}
		handleEvents();
	}
	cleanupSockets();
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
	pollfd clientPollfd = {clientSocket, POLLIN | POLLOUT, 0};
	_fds.push_back(clientPollfd);
	_nfds++;

	// _clients[clientSocket] = new Client(clientSocket);

	std::cout << "Accepted new connection from client." << std::endl;
	return (0);
}

// int clientSocket = _fds[index].fd;
// Client* client = _clients[clientSocket];
// client->readRequest();
// client->processRequest();
// client->sendResponse();
int Server::existingClient(int index)
{
	std::string request;
	if (readRequest(index, request) != 0)
		return (1);
	std::cout << "Received request: " << request << std::endl;
	handleRequest(_fds[index].fd, request);
	// cookie and multiple cgi management.. do bonus or skip?
	return (0);
}

int Server::readRequest(int index, std::string &request)
{
	char	buffer[1024];
	int		bytesRead;

	for (;;)
	{
		bytesRead = recv(_fds[index].fd, buffer, sizeof(buffer) - 1, 0);
		if (bytesRead > 0)
		{
			buffer[bytesRead] = '\0';
			request.append(buffer, bytesRead);
			if (request.find("\r\n\r\n") != std::string::npos)
				break;
		}
		else if (bytesRead == 0) // connection closed by client
		{
			closeClient(index);
			return (1);
		}
		else // recv returned -1
		{
			perror("recv");
			closeClient(index);
			return (1);
		}
	}
	return (0);
}

void Server::handleRequest(int clientSocket, const std::string &requestHTTP)
{
	HttpParser	parser;
	HttpRequest	request;

	request = parser.parseRequest(requestHTTP);
	// Handle different HTTP methods
	if (request.method == "GET")
		handleGET(clientSocket, request.uri);
	else if (request.method == "POST")
		handlePOST(clientSocket, request.uri, request.body);
	else if (request.method == "DELETE")
		handleDELETE(clientSocket, request.uri);
	else
		sendResponse(clientSocket, "405 Method Not Allowed", "text/plain", "405 Method Not Allowed");
}

void Server::closeClient(int index)
{
	close(_fds[index].fd);
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

void Server::sendResponse(int clientSocket, const std::string &status, const std::string &contentType, const std::string &body) const
{
	std::ostringstream response; //no idea if this is compliant
	std::string responseStr;

	response << "HTTP/1.1 " << status << "\r\n";
	response << "Content-Type: " << contentType << "\r\n";
	response << "Content-Length: " << body.size() << "\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	response << body;

	responseStr = response.str();
	send(clientSocket, responseStr.c_str(), responseStr.size(), 0);

std::cout << "Sent response: " << responseStr << std::endl;
}


void Server::handleGET(int clientSocket, const std::string &path)
{
	const std::string &root = currentConfig.getRoot();
// Debug: Print the file path being accessed
std::cout << "path: " << path << " root:" << root << std::endl;
	std::string filePath = root + path;
	if (filePath.back() == '/') filePath += "index.html";
// Debug: Print the file path being accessed
std::cout << "Trying to access file: " << filePath << std::endl;
	std::ifstream file(filePath, std::ios::binary);
	if (!file)
	{
		sendResponse(clientSocket, "404 Not Found", "text/plain", "404 Not Found");
		return;
	}

	std::stringstream buffer;
	buffer << file.rdbuf(); // Read file contents

	// // Get the file extension
	// std::string extension;
	// size_t dotPos = filePath.find_last_of('.');
	// if (dotPos != std::string::npos) {
	// 	extension = filePath.substr(dotPos);
	// }

	// // Get the MIME type based on the file extension
	// std::string mimeType = MimeTypes::getMimeType(extension);
	sendResponse(clientSocket, "200 OK", "text/html", buffer.str());
}

void Server::handlePOST(int clientSocket, const std::string &path, const std::string &body)
{
	if (path == "/submit") {
		std::cout << "Received POST request with body: " << body << std::endl;
		sendResponse(clientSocket, "200 OK", "text/plain", "Data received successfully");
	} else {
		sendResponse(clientSocket, "404 Not Found", "text/plain", "404 Not Found");
	}
}

void Server::handleDELETE(int clientSocket, const std::string &path)
{
	std::string filePath = "." + path;
	
	if (std::remove(filePath.c_str()) == 0) {
		sendResponse(clientSocket, "200 OK", "text/plain", "File deleted successfully");
	} else {
		sendResponse(clientSocket, "404 Not Found", "text/plain", "File not found");
	}
}
