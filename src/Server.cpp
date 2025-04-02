/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 12:02:13 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/26 16:52:14 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server() : _proto(NULL)
{
	//getprotobyname
	_proto = getprotobyname("tcp");
	if (!_proto)
		throw std::runtime_error(std::string("getprotobyname: ") + strerror(errno));
}

Server::Server(const ConfigFileServer &config) : _hostPort(config.getHostPort()), _proto(NULL), _currentConfig(config)
{
	_proto = getprotobyname("tcp");
	if (!_proto)
		throw std::runtime_error(std::string("getprotobyname: ") + strerror(errno));
}

Server::Server(Server const &src) : _hostPort(src._hostPort), _fds(src._fds), _currentConfig(src._currentConfig)  //must finish 
{
	this->_proto = getprotobyname("tcp");
	if (!this->_proto)
		throw std::runtime_error(std::string("getprotobyname: ") + strerror(errno));
}

Server &Server::operator=(Server const &rhs)
{
	if (this != &rhs)
	{
		this->_currentConfig = rhs._currentConfig;
		this->_fds = rhs._fds;
		this->_hostPort = rhs._hostPort;
		this->_proto = getprotobyname("tcp");
		if (!this->_proto)
			throw std::runtime_error(std::string("getprotobyname: ") + strerror(errno));
	}
	return (*this);
}

Server::~Server(void){}

const ConfigFileServer& Server::getConfig() const
{
	return (_currentConfig);
}

int Server::getAddressProtocol(const std::string &host)
{
	struct addrinfo serverAddr = {};
	struct addrinfo *list;
	serverAddr.ai_family = AF_UNSPEC;
	serverAddr.ai_socktype = SOCK_STREAM;
	serverAddr.ai_flags = AI_PASSIVE;
	
	if (getaddrinfo(host.c_str(), NULL, &serverAddr, &list) != 0)
	{
		throw std::runtime_error("getaddrinfo failed for: " + host);
	}
	
	int protocol = list->ai_family;
	freeaddrinfo(list);
	return (protocol);
}


void Server::sockets()
{
	try
	{
		for (size_t i = 0; i < _hostPort.size(); ++i)
		{
			const std::string &host = _hostPort[i].first;
			int port = _hostPort[i].second;
			int protocol = getAddressProtocol(host);
			int serverSocket = createSocket(protocol);
			configureSocket(serverSocket);
			bindAndListen(serverSocket, host, port);
			addToFDList(serverSocket);
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		throw std::runtime_error("Sockets function fails");
	}
	
}

int Server::createSocket(int protocol)
{
	int fd = socket(protocol, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (fd == -1)
		throw std::runtime_error(std::string("socket fnct: ") + strerror(errno));
	return (fd);
}

//  Handling Errors and Disconnections
// if ((_pollFds[i].revents & POLLERR) || _pollFds[i].revents & POLLHUP)
//     _pruneSocket(sd, sS);

void Server::configureSocket(int serverSocket)
{
	int	opt = 1;

	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
	{
		close(serverSocket);
		throw std::runtime_error(std::string("setsockopt: ") + strerror(errno));
	}
	if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) == 1)
	{
		close(serverSocket);
		throw std::runtime_error(std::string("fcntl: ") + strerror(errno));
	}
	setCloexecFlag(serverSocket);
}

std::string intToString(int number)
{
	std::ostringstream oss;
	oss << number;
	return (oss.str());
}

void Server::bindAndListen(int serverSocket, const std::string &host, int port)
{
	struct addrinfo serverAddr = {};
	struct addrinfo *list, *it;

	serverAddr.ai_family = AF_UNSPEC;
	serverAddr.ai_socktype = SOCK_STREAM;
	serverAddr.ai_flags = AI_PASSIVE;

	std::string portStr = intToString(port);
	if (getaddrinfo(host.c_str(), portStr.c_str(), &serverAddr, &list) != 0)
		throw std::runtime_error("getaddrinfo failed for " + host + ":" + portStr); //close socket before throw?
	for (it = list; it != NULL; it = it->ai_next)
		if (bind(serverSocket, it->ai_addr, it->ai_addrlen) == 0)
			break;
	freeaddrinfo(list);
	if (!it)
		throw std::runtime_error("bind failed: " + std::string(strerror(errno))); //close socket before throw?
	if (listen(serverSocket, SOMAXCONN) == -1)
		throw std::runtime_error("listen failed: " + std::string(strerror(errno))); //close socket before throw?
	std::cout << "Server listening on " << host << ":" << port << std::endl;
}

void Server::addToFDList(int serverSocket)
{
	int serverPollfd = serverSocket;
	_fds.push_back(serverPollfd);
}

void Server::addSocketsToEpoll(EPoll &epollInstance)
{
	for (size_t i = 0; i < _fds.size(); ++i)
	{
		int serverFd = _fds[i];
		epollInstance.addFD(serverFd, EPOLLIN);
	}
}

const std::vector<int> &Server::getSockets() const
{
	return (_fds);
}

int Server::acceptClient(int serverFd)
{
	int					clientSocket;
	sockaddr_storage	clientAddr;
	socklen_t			socketSize = sizeof(clientAddr);

	clientSocket = accept(serverFd, (struct sockaddr*)&clientAddr, &socketSize);
	if (clientSocket == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return (-1);
		else
			return (std::cerr << "[ERROR] accept failed: " << strerror(errno) << std::endl, -1);
	}
	fcntl(clientSocket, F_SETFL, O_NONBLOCK);
	return (clientSocket);
}
