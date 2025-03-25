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

Client::Client(void) : _sessionManager(*(new SessionManagement())) {} //tbd??

Client::Client(int socket, const ConfigFileServer &config, SessionManagement &sessionManager) : _clientSocket(socket), _bytesSent(0), _currentConfig(config), _sessionManager(sessionManager) { setCloexecFlag(_clientSocket); }

Client::Client(Client const &src) : _clientSocket(src._clientSocket), _requestBuffer(src._requestBuffer), _responseBuffer(src._responseBuffer), _bytesSent(src._bytesSent), _currentConfig(src._currentConfig), _sessionManager(src._sessionManager)
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

Client::~Client(){}

const ConfigFileServer& Client::getConfigFileServer() const { return _currentConfig; }
void Client::setConfigFileServer(const ConfigFileServer &config) { _currentConfig = config; }

HTTPRequest Client::readRequest()  //check return values to kill process??
{
	HTTPRequest	http;
	char		buffer[1024];
	int			bytesRead;
	bool		headersRead = false;

	while (true)
	{
		bytesRead = recv(_clientSocket, buffer, sizeof(buffer) - 1, 0);
		if (bytesRead > 0)
		{
			buffer[bytesRead] = '\0';
			_requestBuffer.append(buffer, bytesRead);

			if (!headersRead && _requestBuffer.find("\r\n\r\n") != std::string::npos)
			{
				headersRead = true;
				try
				{
					http.parserHeaders(_requestBuffer);
				}
				catch (const std::runtime_error &e)
				{
					serveErrorResponse(400);
					return (http);
				}
				if (http.request.contentLength == 0 && http.request.headers.find("Transfer-Encoding") == http.request.headers.end())
					break;
			}
			if (headersRead && (lengthData(http) || chunkedData(http)))
				break;
		}
		else if (bytesRead == 0)
		{
			closeClient();
			break;
		}
		else
			break; // if i closeClient or send empty httprequest it doesnt work
	}
	if (http.request.headers.find("Connection") != http.request.headers.end() && http.request.headers["Connection"] == "close")
		closeClient();
	return (http);
}

bool Client::chunkedData(HTTPRequest &http)
{
	if (http.request.headers.find("Transfer-Encoding") != http.request.headers.end() &&
		http.request.headers["Transfer-Encoding"] == "chunked")
	{
		std::string body;
		size_t headerEnd = _requestBuffer.find("\r\n\r\n") + 4;
		while (true)
		{
			size_t chunkEnd = _requestBuffer.find("\r\n", headerEnd);
			if (chunkEnd == std::string::npos)
				return (false); // keep reading
			std::string chunkSizeStr = _requestBuffer.substr(headerEnd, chunkEnd - headerEnd);
			size_t chunkSize = stringTUL(chunkSizeStr);
			headerEnd = chunkEnd + 2;
			if (chunkSize == 0)
				break; // finish reading
			if (headerEnd + chunkSize > _requestBuffer.size())
				return (false); // keep reading
			body.append(_requestBuffer.substr(headerEnd, chunkSize));
			headerEnd += chunkSize + 2;
		}
		http.request.body = body;
		return (true); // everything read
	}
	return (false);
}

bool Client::lengthData(HTTPRequest &http)
{
	if (http.request.contentLength == 0)
		return (true);

	size_t contentLength = http.request.contentLength;
	size_t headersEndPos = _requestBuffer.find("\r\n\r\n") + 4;
	if (_requestBuffer.size() - headersEndPos >= contentLength)
		return (http.parserBody(_requestBuffer.substr(headersEndPos, contentLength)), true); // everything read
	return (false); //keep reading
}

bool Client::isMethodAllowed(const ConfigFileServerLocation *location, const std::string &method)
{
	if (location)
	{
		const std::vector<std::string> &methods = location->getAllowedMethods();
		if (!methods.empty() && std::find(methods.begin(), methods.end(), method) == methods.end())
			return (false);
	}
	return (true);
}

void Client::handleCookies(HTTPRequest &http)
{
	if (http.request.headers.find("Cookie") != http.request.headers.end())
		_cookies.parse(http.request.headers["Cookie"]);
	std::string sessionID = _cookies.getCookie("SESSIONID");
	// std::cout << " FIRST " << sessionID << std::endl;
	if (!sessionID.empty() && !_sessionManager.sessionExists(sessionID))
	{
		_sessionManager.createSession(sessionID);
		_cookies.setCookie("SESSIONID", sessionID + "; Path=/; HttpOnly");
		// std::cout << sessionID << std::endl;
	}
	else if (sessionID.empty()) 
	{
		sessionID = _sessionManager.createSession("");
		_cookies.setCookie("SESSIONID", sessionID);
	} 
	std::map<std::string, std::string> &session = _sessionManager.getSession(sessionID);
	session["last_access"] = getCurrentTimestamp();
}

void Client::handleRequest(HTTPRequest &http)
{	
	http.logRequest(getCurrentTimestamp());

	handleCookies(http);
	// DO WE NEED TO PARSE FOR VALID HOSTNAMES???? and for $ variables?
	if (!validateRequest(http))
		return;
	
	try 
	{
		if (routeToCGI(http.resolvedFilePath))
		{
			CGI cgi(_currentConfig);
			cgi.execute(&http);
			std::string cgiHeaders = cgi.getHeaders();
			std::string setCookieHeaders = _cookies.generateSetCookieHeader();
			if (!setCookieHeaders.empty())
				cgiHeaders += setCookieHeaders;
			prepareResponse(200, "text/html", cgi.getOutput(), "", cgiHeaders);
			return;
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		serveErrorResponse(500);
		return;
	}
	if (http.request.method == "GET")
		handleGET(&http);
	else if (http.request.method == "POST")
		handlePOST(&http);
	else if (http.request.method == "DELETE")
		handleDELETE(&http);
	else
		serveErrorResponse(405);

	resetState();
}

bool Client::validateRequest(HTTPRequest &http)
{
	const ConfigFileServerLocation *matchedLocation = matchLocation(http);

	if (locationReturn(matchedLocation))
		return (false);
	if (serverReturn())
		return (false);

	if (matchedLocation && !isMethodAllowed(matchedLocation, http.request.method))
		return (serveErrorResponse(405), false);

	if (http.request.contentLength > _currentConfig.getMaxBodySize())
		return (serveErrorResponse(413), false);

	http.resolvedFilePath = http.resolveFilePath(_currentConfig);
	if (http.resolvedFilePath.empty())
		return (serveErrorResponse(404), false);
	return (true);
}

void Client::resetState()
{
	_requestBuffer.clear();
	_bytesSent = 0;
}

bool Client::routeToCGI(std::string requestURI)
{
	size_t dotPos = requestURI.find_last_of('.');
	if (dotPos == std::string::npos)
		return (false);
	std::string extension = requestURI.substr(dotPos);
	if (extension == ".php" || extension == ".py")
		return (true);
	return (false);
};

const ConfigFileServerLocation* Client::matchLocation(const HTTPRequest &http) const
{
	const ConfigFileServerLocation *bestLocation = NULL;
	const std::map<std::string, ConfigFileServerLocation> &locations = _currentConfig.getLocations();
	for (std::map<std::string, ConfigFileServerLocation>::const_iterator it = locations.begin(); it != locations.end(); ++it)
		if (http.request.uri.find(it->first) == 0 && (bestLocation == NULL || it->first.length() > bestLocation->getURI().length()))
			bestLocation = &it->second;
	return (bestLocation);
}

bool Client::handleReturnDirective(int statusCode, const std::string &redirectUrl)
{
	if (ErrorPage::isErrorStatusCode(statusCode))
	{
		serveErrorResponse(statusCode);
		return (true);
	}
	std::string body = "Redirecting to " + redirectUrl;
	prepareResponse(statusCode, "text/plain", body, redirectUrl, "");
	return (true);
}

bool Client::locationReturn(const ConfigFileServerLocation *location)
{
	if (location && location->hasReturnDirective())
	{
		int statusCode = location->getReturnStatusCode();
		std::string redirect = location->getReturnUrl();
		return (handleReturnDirective(statusCode, redirect));
	}
	return (false);
}

bool Client:: serverReturn(void)
{
	if (_currentConfig.hasReturnDirective())
	{
		int statusCode = _currentConfig.getReturnStatusCode();
		std::string redirect = _currentConfig.getReturnUrl();
		return (handleReturnDirective(statusCode, redirect));
	}
	return (false);
}

void Client::closeClient()
{
	if (_clientSocket != -1) // Ensure it's valid before closing
	{
		close(_clientSocket);
		_clientSocket = -1; // Mark as closed
		// std::cout << "[DEBUG] Client FD closed: " << _clientSocket << std::endl;
	}
}

int Client::getSocket() const
{
	return (_clientSocket);
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
			continue;
		if (written == 0)
		{
			closeClient(); // connection closed by client
			return;
		}
		sent += written;
		_bytesSent += written;
	}
	if (_bytesSent >= total)
	{
		if (_requestBuffer.find("Connection: keep-alive") != std::string::npos)
		{
			resetState();
			_keepAlive = true;
		}
		else
			closeClient();
	}
}

bool Client::keepAlive() const
{
	return (_keepAlive);
}

void Client::handleGET(HTTPRequest	*http)
{
	std::string filePath = http->resolveFilePath(_currentConfig);
	// std::cout << "FILE PATH : " << filePath << std::endl;
	if (filePath == "403")
	{
		serveErrorResponse(403);
		return;
	}

	struct stat pathStat;
	if (stat(filePath.c_str(), &pathStat) == 0 && S_ISDIR(pathStat.st_mode))
	{
		std::string directoryListing = HTTPResponse::directoryList(filePath, http->request.uri);
		prepareResponse(200, "text/html", directoryListing, "", "");
		return;
	}

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
	std::string mimeType = getMimeType(fileExtension);
	prepareResponse(200, mimeType, body, "", "");
}

void Client::handlePOST(HTTPRequest *http)
{
	std::string root = http->resolveFilePath(_currentConfig);

	if (http->request.headers.find("Content-Type") != http->request.headers.end() &&
		http->request.headers["Content-Type"].find("multipart/form-data") != std::string::npos)
	{
		std::string boundary = http->request.headers["Content-Type"].substr(http->request.headers["Content-Type"].find("boundary=") + 9);
		boundary.erase(boundary.find_last_not_of(" \t\r\n") + 1);

		std::string fullBoundary = "--" + boundary;
		std::string closingBoundary = fullBoundary + "--";

		size_t start = http->request.body.find(fullBoundary);
		if (start == std::string::npos)
		{
			serveErrorResponse(400);//Starting boundary not found");
			return;
		}
		start += fullBoundary.length() + 2;

		size_t end = http->request.body.find("\r\n" + closingBoundary, start);
		if (end == std::string::npos)
			end = http->request.body.find("\n" + closingBoundary, start);
		if (end == std::string::npos)
			end = http->request.body.find(closingBoundary, start);
		if (end == std::string::npos)
		{
			serveErrorResponse(400);//Bad Request: Ending boundary not found");
			return;
		}
		std::string part = http->request.body.substr(start, end - start);
		size_t headerEnd = part.find("\r\n\r\n");
		if (headerEnd == std::string::npos)
		{
			serveErrorResponse(400);//Could not find headers in part");
			return;
		}
		headerEnd += 4;
		std::string fileContent = part.substr(headerEnd);

		size_t filenamePos = part.find("filename=\"");
		if (filenamePos == std::string::npos)
		{
			serveErrorResponse(400);//Filename not found in Content-Disposition header");
			return;
		}
		filenamePos += 10;
		size_t filenameEnd = part.find("\"", filenamePos);
		if (filenameEnd == std::string::npos)
		{
			serveErrorResponse(400);//Invalid filename in Content-Disposition header");
			return;
		}
		std::string filename = part.substr(filenamePos, filenameEnd - filenamePos);
		std::string filePath = root + "/" + filename;

		std::ofstream file(filePath.c_str(), std::ios::binary);
		if (!file)
		{
			serveErrorResponse(500); //SERVER Error or BAD REQUEST?
			return;
		}
		file << fileContent;
		file.close();
	}
	else
	{
		std::string filename = http->request.headers["X-Filename"];
		filename.erase(std::remove(filename.begin(), filename.end(), '\r'), filename.end());
		filename.erase(std::remove(filename.begin(), filename.end(), '\n'), filename.end());
		filename.erase(std::remove(filename.begin(), filename.end(), '\''), filename.end());
		if (filename.empty())
		{
			serveErrorResponse(400);//prepareErrorResponse(400, "text/plain", "400 Bad Request: Filename header missing");
			return;
		}
		std::string filePath = root + "/" + filename;
		std::ofstream file(filePath.c_str(), std::ios::binary);
		if (!file)
		{
			serveErrorResponse(500);
			return;
		}
		file << http->request.body;
		file.close();
	}
	prepareResponse(200, "text/plain", "File uploaded successfully\n", "", "");
}

void Client::handleDELETE(HTTPRequest *http)
{
	std::string root = http->resolveFilePath(_currentConfig);
	std::string filename = http->request.headers["X-Filename"];
	filename.erase(std::remove(filename.begin(), filename.end(), '\r'), filename.end());
	filename.erase(std::remove(filename.begin(), filename.end(), '\n'), filename.end());
	filename.erase(std::remove(filename.begin(), filename.end(), '\''), filename.end());
	if (filename.empty())
	{
		serveErrorResponse(400);//prepareErrorResponse(400, "text/plain", "400 Bad Request: Filename header missing");
		return;
	}
	std::string filePath = root + "/" + filename;
	if (std::remove(filePath.c_str()) == 0)
		prepareResponse(200, "text/plain", "File deleted successfully", "", "");
	else
		serveErrorResponse(404);
}

void Client::prepareResponse(int statusCode, const std::string &contentType, const std::string &body, const std::string &redirect = "", const std::string &additionalHeaders = "")
{
	if (ErrorPage::isErrorStatusCode(statusCode))
	{
		serveErrorResponse(statusCode);
		return;
	}
	HTTPResponse response;
	response.setResponse(statusCode, contentType, body, redirect, additionalHeaders);
	_responseBuffer = response.toString();
	response.logResponse(getCurrentTimestamp());
}

void Client::prepareErrorResponse(int statusCode, const std::string &contentType, const std::string &body)
{
	HTTPResponse response;
	std::stringstream ss;
	ss << body.size();
	response.setStatus(statusCode)
			.setHeader("Content-Type", contentType)
			.setHeader("Content-Length", ss.str())
			.setBody(body);

	_responseBuffer = response.toString();
	response.logResponse(getCurrentTimestamp());
}

void Client::serveErrorResponse(int statusCode)
{
	const std::map<int, std::string> &errorPages = _currentConfig.getErrorPages();
	std::map<int, std::string>::const_iterator it = errorPages.find(statusCode);

	if (it != errorPages.end())
	{
		std::string errorPagePath = it->second;
		std::ifstream file(errorPagePath.c_str(), std::ios::binary);
		if (file)
		{
			std::stringstream buffer;
			buffer << file.rdbuf();
			std::string body = buffer.str();
			prepareErrorResponse(statusCode, "text/html", body);
			resetState();
			return;
		}
		else
			std::cerr << "[ERROR] Failed to open custom error page: " << errorPagePath << std::endl;
	}

	std::string errorPage = ErrorPage::generate(statusCode);
	std::ifstream file(errorPage.c_str(), std::ios::binary);
	if (file)
	{
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string body = buffer.str();
		prepareErrorResponse(statusCode, "text/html", body);
		ErrorPage::cleanup(errorPage);
	}
	else
	{
		std::string body = "<html><head><title>500 Internal Server Error</title></head>"
						"<body><h1>500 Internal Server Error</h1>"
						"<p>Something went wrong. Please try again later.</p></body></html>";
		prepareErrorResponse(500, "text/html", body);
	}
	resetState();
}
