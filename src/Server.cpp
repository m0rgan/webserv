/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 12:02:13 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/23 17:20:41 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server() : _nfds(0), _proto(NULL)
{
	//getprotobyname
	_proto = getprotobyname("tcp");
	if (!_proto)
		throw std::runtime_error(std::string("getprotobyname: ") + strerror(errno));
}

Server::Server(const ServerConfig &config) : _ports(config.getPorts()), _hosts(config.getHosts()), _nfds(0), _proto(NULL), _currentConfig(config)
{
	_proto = getprotobyname("tcp");
	if (!_proto)
		throw std::runtime_error(std::string("getprotobyname: ") + strerror(errno));
}

Server::Server(Server const &src) : _ports(src._ports), _hosts(src._hosts), _fds(src._fds), _nfds(src._nfds), _currentConfig(src._currentConfig)  //must finish 
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
		this->_nfds = rhs._nfds;
		this->_fds = rhs._fds;
		this->_ports = rhs._ports;
		this->_proto = getprotobyname("tcp");
		if (!this->_proto)
			throw std::runtime_error(std::string("getprotobyname: ") + strerror(errno));
	}
	return (*this);
}

Server::~Server(void){}

const ServerConfig& Server::getConfig() const
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
		std::cerr << "getaddrinfo failed for " << host << std::endl;
		return -1;
	}

	int protocol = list->ai_family;
	freeaddrinfo(list);
	return (protocol);
}


int Server::sockets()
{
	int serverSocket;
	size_t i;

	for (i = 0; i < _ports.size(); ++i)
	{
		int protocol = getAddressProtocol(_hosts[i]);
		if (protocol == -1)
		{
			std::cerr << "Invalid address protocol for " << _hosts[i] << std::endl;
			return (1);
		}

		serverSocket = createSocket(protocol);
		if (serverSocket < 0)
		{
			std::cerr << "[ERROR] Failed to create socket!" << std::endl;
			return (1);
		}
		
		if (configureSocket(serverSocket))
		{
			std::cerr << "[ERROR] Failed to configure socket!" << std::endl;
			return (1);
		}
		if (bindAndListen(serverSocket, _hosts[i], _ports[i]))
		{
			std::cerr << "[ERROR] Failed to bind and listen on " << _hosts[i] << ":" << _ports[i] << std::endl;
			return (1);
		}
		addToPollList(serverSocket);
	}

	return 0;
}


int Server::createSocket(int protocol)
{
	int fd = socket(protocol, SOCK_STREAM, 0);
	if (fd == -1)
		throw std::runtime_error(std::string("socket: ") + strerror(errno));
	return fd;
}


//  Handling Errors and Disconnections
// if ((_pollFds[i].revents & POLLERR) || _pollFds[i].revents & POLLHUP)
//     _pruneSocket(sd, sS);

int Server::configureSocket(int serverSocket)
{
	int	opt;

	opt = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		std::cerr << "setsockopt: " << strerror(errno) << std::endl;
		close(serverSocket);
		return (1);
	}
	if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) == 1)
	{
		std::cerr << "fcntl: " << strerror(errno) << std::endl;
		close(serverSocket);
		return (1);
	}
	return (0);
}

std::string intToString(int number)
{
	std::ostringstream oss;
	oss << number;
	return (oss.str());
}


int Server::bindAndListen(int serverSocket, const std::string &host, int port)
{
	struct addrinfo serverAddr = {};
	struct addrinfo *list, *it;

	serverAddr.ai_family = AF_UNSPEC;
	serverAddr.ai_socktype = SOCK_STREAM;
	serverAddr.ai_flags = AI_PASSIVE;

	std::string portStr = intToString(port);
	if (getaddrinfo(host.c_str(), portStr.c_str(), &serverAddr, &list) != 0)
		return (std::cerr << "getaddrinfo failed for " << host << ":" << port << std::endl, 1);
	for (it = list; it != NULL; it = it->ai_next)
		if (bind(serverSocket, it->ai_addr, it->ai_addrlen) == 0)
			break;
	freeaddrinfo(list);
	if (!it)
		return (std::cerr << "bind: " << strerror(errno) << std::endl, close(serverSocket), 1);
	if (listen(serverSocket, SOMAXCONN) == -1)
		return (std::cerr << "listen: " << strerror(errno) << std::endl, close(serverSocket), 1);
	std::cout << "Server listening on " << host << ":" << port << std::endl;
	return 0;
}


void Server::addToPollList(int serverSocket)
{
	pollfd serverPollfd = {serverSocket, POLLIN, 0};
	_fds.push_back(serverPollfd);
	_nfds++;
}

const std::string Server::getName() const {return (_currentConfig.getName());};
const std::vector<pollfd> &Server::getSockets() const {return (_fds);}
const std::vector<int> Server::getPorts() const {return (_ports);};
const std::vector<std::string> Server::getHosts() const {return (_hosts);};

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
