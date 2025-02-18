/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 17:00:36 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/15 17:00:36 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(void) {} //tbd??

Client::Client(int socket, const ServerConfig &config) : _clientSocket(socket), _bytesSent(0), _currentConfig(config)
{
	fcntl(_clientSocket, F_SETFL, O_NONBLOCK);
}

Client::Client(Client const &src)  //must finish
{
	this->_clientSocket = src._clientSocket;
	return;
}

Client &Client::operator=(Client const &rhs) //must finish
{
	if (this != &rhs)
		return (*this);
	return (*this);
}

Client::~Client()
{
	close(_clientSocket);
	//delete client object??
std::cout << "Closed connection with client: " << _clientSocket << std::endl;
}

int Client::getSocket() const
{
	return (_clientSocket);
}

void Client::readRequest() //check return values to kill process??
{
	char	buffer[1024];
	int		bytesRead;

	for (;;)
	{
		bytesRead = recv(_clientSocket, buffer, sizeof(buffer) - 1, 0);
		if (bytesRead > 0)
		{
			buffer[bytesRead] = '\0';
			_requestBuffer.append(buffer, bytesRead);
			if (_requestBuffer.find("\r\n\r\n") != std::string::npos)
				break;
		}
		else if (bytesRead == 0) // connection closed by client
			throw std::runtime_error("Connection closed by client");
		else // recv returned -1
		{
			perror("recv");
			throw std::runtime_error("Error reading request");
		}
	}
}

void Client::handleRequest()
{
	HttpParser	parser;
	HttpRequest	request;

	request = parser.parseRequest(_requestBuffer);
	if (request.method == "GET")
		handleGET(request.uri);
	else if (request.method == "POST")
		handlePOST(request.uri, request.body);
	else if (request.method == "DELETE")
		handleDELETE(request.uri);
	else
		prepareResponse("405 Method Not Allowed", "text/plain", "405 Method Not Allowed");
}

void Client::closeClient()
{
	close(_clientSocket);
}

void Client::prepareResponse(const std::string &status, const std::string &contentType, const std::string &body)
{
	std::ostringstream responseStream;
	responseStream << "HTTP/1.1 " << status << "\r\n";
	responseStream << "Content-Type: " << contentType << "\r\n";
	responseStream << "Content-Length: " << body.length() << "\r\n";
	responseStream << "Connection: close\r\n";
	responseStream << "\r\n";
	responseStream << body;

	_responseBuffer = responseStream.str();
	_bytesSent = 0;
	// _readyToSend = true;

	// for (size_t i = 0; i < _fds.size(); ++i)
	// {
	// 	if (_fds[i].fd == _clientSocket)
	// 	{
	// 		_fds[i].events |= POLLOUT;  // POLLOUT only when response is ready
	// 		break;
	// 	}
	// }
}

void Client::prepareBinaryResponse(const std::string &status, const std::string &contentType, const std::vector<char> &body)
{
	std::ostringstream responseStream;
	responseStream << "HTTP/1.1 " << status << "\r\n";
	responseStream << "Content-Type: " << contentType << "\r\n";
	responseStream << "Content-Length: " << body.size() << "\r\n";
	responseStream << "Connection: keep-alive\r\n";
	responseStream << "\r\n";

	std::string headers = responseStream.str();
	_responseBuffer = headers;
	_responseBuffer.append(body.begin(), body.end());
	_bytesSent = 0;
}


bool Client::hasPendingData() const
{
std::cout << "bytesSent = " << _bytesSent << ", responseBuffer.length() = " << _responseBuffer.length() << std::endl;
	return (_bytesSent < _responseBuffer.length());
}


void Client::writeResponse()
{
	const char *responseData = _responseBuffer.c_str() + _bytesSent;
	ssize_t bytesToSend = _responseBuffer.length() - _bytesSent;
	ssize_t bytesSentNow;

	while (bytesToSend > 0)
	{
std::cout << "Attempting to send: bytesToSend = " << bytesToSend << std::endl;
		bytesSentNow = send(_clientSocket, responseData, bytesToSend, 0);
		if (bytesSentNow == -1)
		{
			perror("send");
			throw std::runtime_error("Error sending response");
		}
		_bytesSent += bytesSentNow;
		bytesToSend -= bytesSentNow;
		responseData += bytesSentNow;
std::cout << "Sent " << bytesSentNow << " bytes (Total: " << _bytesSent << " / " << _responseBuffer.length() << ")" << std::endl;
	}

	if (_bytesSent == _responseBuffer.length())
	{
std::cout << "All data sent, CLOSING CLIENT." << std::endl;
std::cout << "Sent " << bytesSentNow << " bytes (Total: " << _bytesSent << " / " << _responseBuffer.length() << ")" << std::endl;
		closeClient();
	}
}

std::string Client::getMimeType(const std::string &extension) const //should this be a member fnc?
{
	static std::map<std::string, std::string> mimeTypes;
	if (mimeTypes.empty())
	{
		mimeTypes[".html"] = "text/html";
		mimeTypes[".css"] = "text/css";
		mimeTypes[".js"] = "application/javascript";
		mimeTypes[".png"] = "image/png";
		mimeTypes[".jpg"] = "image/jpeg";
		mimeTypes[".jpeg"] = "image/jpeg";
		mimeTypes[".gif"] = "image/gif";
		mimeTypes[".svg"] = "image/svg+xml";
		mimeTypes[".ico"] = "image/x-icon";
		// are there other MIME types?
	}
	std::map<std::string, std::string>::const_iterator it = mimeTypes.find(extension);
	if (it != mimeTypes.end())
		return (it->second);
	return ("application/octet-stream");
}

void Client::handleGET(const std::string &path)
{
	const std::string &root = _currentConfig.getRoot();
std::cout << "path: " << path << " root:" << root << std::endl;
	std::string filePath = root + path;
	if (filePath.back() == '/')
		filePath += "index.html";
std::cout << "Trying to access file: " << filePath << std::endl;
	std::ifstream file(filePath, std::ios::binary);
	if (!file)
	{
		prepareResponse("404 Not Found", "text/plain", "404 Not Found");
		return;
	}

	// std::stringstream buffer;
	// buffer << file.rdbuf();
	// std::string body = buffer.str();
	// std::cout << "File content size: " << body.size() << std::endl;
	std::vector<char> body((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	std::string extension;
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos != std::string::npos)
		extension = filePath.substr(dotPos);
	std::string mimeType = getMimeType(extension);
	prepareBinaryResponse("200 OK", mimeType, body);
}

void Client::handlePOST(const std::string &path, const std::string &body)
{
	if (path == "/submit")
	{
		std::cout << "Received POST request with body: " << body << std::endl;
		prepareResponse("200 OK", "text/plain", "Data received successfully");
	}
	else
		prepareResponse("404 Not Found", "text/plain", "404 Not Found");
}

void Client::handleDELETE(const std::string &path)
{
	std::string filePath = "." + path;
	
	if (std::remove(filePath.c_str()) == 0)
		prepareResponse("200 OK", "text/plain", "File deleted successfully");
	else
		prepareResponse("404 Not Found", "text/plain", "File not found");
}
