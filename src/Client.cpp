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

Client::Client(Client const &src) : _clientSocket(src._clientSocket), _requestBuffer(src._requestBuffer), _responseBuffer(src._responseBuffer), _bytesSent(src._bytesSent), _currentConfig(src._currentConfig)
{
	return;
}

Client &Client::operator=(Client const &rhs) //must finish
{
	if (this != &rhs)
	{
		this->_clientSocket = rhs._clientSocket;
		this->_requestBuffer = rhs._requestBuffer;
		this->_responseBuffer = rhs._responseBuffer;
		this->_bytesSent = rhs._bytesSent;
		this->_currentConfig = rhs._currentConfig;
	}
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
			throw std::runtime_error(std::string("Connection closed by client: ") + strerror(errno));
		else // recv returned -1
			throw std::runtime_error(std::string("recv: error reading request: ") + strerror(errno));
	}
}

void Client::handleRequest()
{
	HTTPRequest	http; // constructor that parses?
	CGI cgi;

	http.parser(_requestBuffer);

	try 
	{
		 if (cgi.routeToCGI(http.request.uri))
		 {
			cgi.execute(http);
			return;
		 }
	}
	catch (const std::exception &e)
	{
		std::cerr << "CGI error " << e.what() << std::endl;
		prepareResponse("500 Internal Server Error", "text/plain", "500 Internal Server Error");
		return;
	}
	if (http.request.method == "GET")
		handleGET(http.request.uri);
	else if (http.request.method == "POST")
		handlePOST(http.request.uri, http.request.body);
	else if (http.request.method == "DELETE")
		handleDELETE(http.request.uri);
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
}

bool Client::hasPendingData() const
{
	if (_bytesSent < 0)
		return (true);
	return (_bytesSent < static_cast<ssize_t>(_responseBuffer.length()));
}


void Client::writeResponse()
{
	ssize_t totalBytesSent = 0;
	ssize_t bytesJustSent;
	ssize_t bytesToSend = _responseBuffer.size();
	const char *responseData = _responseBuffer.c_str();

	while (totalBytesSent < bytesToSend)
	{
		bytesJustSent = send(_clientSocket, responseData + totalBytesSent, bytesToSend - totalBytesSent, 0);
		// MSG_NOSIGNAL (avoid SIGPIPE crashes)
		// MSG_OOB        0x1  /* process out-of-band data */
		if (bytesJustSent == -1)
			continue; // is this correct? if i cant check errno is this the only option?
		if (bytesJustSent == 0)
		{
			std::cerr << "Connection closed by client." << std::endl;
			closeClient();
			return;
		}
		totalBytesSent += bytesJustSent;
		_bytesSent += bytesJustSent;
	}
	if (_bytesSent >= bytesToSend)
		closeClient();
// 	if (_bytesSent == static_cast<ssize_t>(_responseBuffer.length()))
// {
//     std::cout << "[DEBUG] All data sent. Checking keep-alive..." << std::endl;
//     if (_requestBuffer.find("Connection: keep-alive") != std::string::npos)
//     {
//         std::cout << "[DEBUG] Keeping connection alive for client: " << _clientSocket << std::endl;
//     }
//     else
//     {
//         std::cout << "[DEBUG] Closing client due to no keep-alive." << std::endl;
//         closeClient();
//     }
// }

}

void Client::handleGET(const std::string &path)
{
	const std::string &root = _currentConfig.getRoot();
	std::string filePath = root + path;
	if (filePath.back() == '/')
		filePath += "index.html";
	// substitute filepath logic for a function manages roots
	std::cout << filePath << std::endl;
	std::ifstream file(filePath, std::ios::binary);
	if (!file)
	{
		prepareResponse("404 Not Found", "text/plain", "404 Not Found");
		return;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string body = buffer.str();

	std::string fileExtension;
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos != std::string::npos)
		fileExtension = filePath.substr(dotPos);
	Utilities utils;
	std::string mimeType = utils.getMimeType(fileExtension);
	prepareResponse("200 OK", mimeType, body);
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
