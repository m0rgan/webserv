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
	fcntl(_clientSocket, F_SETFL, O_NONBLOCK); //is this neccessary?
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
	// Request Processing
	// If it’s a static file request, it locates the file and prepares a response.
	// If it’s a proxy request, it forwards the request to a backend.
	// If it’s a FastCGI request, it communicates with PHP/CGI.
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
		prepareResponse(500, "text/plain", "500 Internal Server Error");
		return;
	}
	if (http.request.method == "GET")
		handleGET(&http);
	else if (http.request.method == "POST")
		handlePOST(http.request.uri, http.request.body);
	else if (http.request.method == "DELETE")
		handleDELETE(http.request.uri);
	else
		prepareResponse(405, "text/plain", "405 Method Not Allowed");
}

void Client::closeClient()
{
	if (_clientSocket != -1) // Ensure it's valid before closing
	{
		close(_clientSocket);
		_clientSocket = -1; // Mark as closed
		std::cout << "[DEBUG] Client FD closed: " << _clientSocket << std::endl;
	}
}

void Client::prepareResponse(int statusCode, const std::string &contentType, const std::string &body)
{
	Response response;
	std::stringstream ss;
	ss << body.size();
	response.setStatus(statusCode)
			.setHeader("Content-Type", contentType)
			.setHeader("Content-Length", ss.str())
			.setBody(body);

	_responseBuffer = response.buildResponse();
}


bool Client::hasPendingData() const
{
	if (_bytesSent < 0)
		return (true);
	return (_bytesSent < static_cast<ssize_t>(_responseBuffer.length()));
}


void Client::writeResponse()
{
	ssize_t sent = 0;
	ssize_t written;
	ssize_t total = _responseBuffer.size();
	const char *responseData = _responseBuffer.c_str();

	while (sent < total)
	{
		written = send(_clientSocket, responseData + sent, total - sent, MSG_NOSIGNAL);
		// MSG_OOB        0x1  /* process out-of-band data */
		if (written == -1)
			continue; // is this correct? if i cant check errno is this the only option?
		if (written == 0)
		{
			std::cerr << "Connection closed by client." << std::endl;
			closeClient();
			return;
		}
		sent += written;
		_bytesSent += written;
	}
	// if (_bytesSent >= total)
		// closeClient();
	// if (_bytesSent >= total)
	// {
	//     send(_clientSocket, "0\r\n\r\n", 5, MSG_NOSIGNAL);
	//     closeClient();
	// }
	// 
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

void Client::handleGET(HTTPRequest	*http)
{
	std::string filePath = http->resolveFilePath(_currentConfig);
	std::ifstream file(filePath.c_str(), std::ios::binary);
	if (!file)
	{
		serveErrorResponse(404);
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
	prepareResponse(200, mimeType, body);
}

void Client::serveErrorResponse(int statusCode)
{
	std::string errorPage = ErrorPage::generate(statusCode);
	std::ifstream file(errorPage.c_str(), std::ios::binary);
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string body = buffer.str();
	prepareResponse(statusCode, "text/html", body);
	ErrorPage::cleanup(errorPage);
}


void Client::handlePOST(const std::string &path, const std::string &body)
{
	if (path == "/submit")
	{
		std::cout << "Received POST request with body: " << body << std::endl;
		prepareResponse(200, "text/plain", "Data received successfully");
	}
	else
		serveErrorResponse(404);
}

void Client::handleDELETE(const std::string &path)
{
	std::string filePath = "." + path;
	
	if (std::remove(filePath.c_str()) == 0)
		prepareResponse(200, "text/plain", "File deleted successfully");
	else
		serveErrorResponse(404);
}
