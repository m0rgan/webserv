/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 17:00:36 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/06/07 16:55:31 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(void)
	: _clientSocket(-1),
	  _bytesSent(0),
	  _currentConfig(ConfigFileServer()),
	  _sessionManager(*(new SessionManagement())),
	  _pendingRequest(NULL),
	  _cgi(NULL),
	  _cgiPipeFD(-1),
	  _serverLauncher(NULL),
	  _keepAlive(false)
{}

Client::Client(int socket, const ConfigFileServer &config, SessionManagement &sessionManager, ServerLauncher* serverLauncher)
	: _clientSocket(socket),
	  _bytesSent(0),
	  _currentConfig(config),
	  _sessionManager(sessionManager),
	  _pendingRequest(NULL),
	  _cgi(NULL),
	  _cgiPipeFD(-1),
	  _serverLauncher(serverLauncher),
	  _keepAlive(false),
	  _requestBuffer(),
	  _responseBuffer()
{
	setCloexecFlag(_clientSocket);
}

Client::Client(Client const &src)
	: _clientSocket(src._clientSocket),
	_bytesSent(src._bytesSent),
	_currentConfig(src._currentConfig),
	_sessionManager(src._sessionManager),
	_pendingRequest(NULL),
	_cgi(NULL),
	_cgiPipeFD(-1),
	_serverLauncher(src._serverLauncher),
	_keepAlive(src._keepAlive),
	_requestBuffer(src._requestBuffer),
	_responseBuffer(src._responseBuffer)
{}

Client &Client::operator=(Client const &rhs)
{
	if (this != &rhs)
	{
		_clientSocket = rhs._clientSocket;
		_bytesSent = rhs._bytesSent;
		_currentConfig = rhs._currentConfig;
		_pendingRequest = NULL;
		_cgi = NULL;
		_cgiPipeFD = -1;
		_serverLauncher = rhs._serverLauncher;
		_keepAlive = rhs._keepAlive;
		_requestBuffer = rhs._requestBuffer;
		_responseBuffer = rhs._responseBuffer;
	}
	return *this;
}

Client::~Client()
{
	if (_cgi)
		delete _cgi;
	if (_pendingRequest)
		delete _pendingRequest;
}

bool Client::isCGIFD(int fd) const {
	return fd == _cgiPipeFD;
}


const ConfigFileServer& Client::getConfigFileServer() const { return _currentConfig; }
void Client::setConfigFileServer(const ConfigFileServer &config) { _currentConfig = config; }
bool Client::keepAlive() const { return (_keepAlive); }

HTTPRequest* Client::readRequest()
{
	HTTPRequest* http = new HTTPRequest();
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
					http->parserHeaders(_requestBuffer);
				}
				catch (const std::runtime_error &e)
				{
					delete http;
					prepareErrorResponse(400);
					throw std::runtime_error("400 Bad Request");
				}
				if (http->contentLength == 0 && http->headers.find("Transfer-Encoding") == http->headers.end())
					break;
			}
			try
			{
				if (headersRead && (lengthData(http) || chunkedData(http)))
					break;
			}
			catch(const std::runtime_error &e)
			{
				delete http;
				prepareErrorResponse(400);
				throw std::runtime_error("400 Bad Request");
			}
		}
		else if (bytesRead == 0)
		{
			_keepAlive = false;
			delete http;
			http = NULL;
			break;
		}
		else
			break;
	}
	if (http && http->headers.find("Connection") != http->headers.end() && http->headers["Connection"] == "close")
		_keepAlive = false;
	return (http);
}

bool Client::chunkedData(HTTPRequest *http)
{
	if (http->headers.find("Transfer-Encoding") != http->headers.end() &&
		http->headers["Transfer-Encoding"] == "chunked")
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
		http->body = body;
		if (http->method == "POST" && http->body.empty())
		{
			throw std::runtime_error("400 Bad Request");
		}
		return (true); // everything read
	}
	return (false);
}

bool Client::lengthData(HTTPRequest *http)
{
	if (http->contentLength == 0)
		return (true);

	size_t contentLength = http->contentLength;
	size_t headersEndPos = _requestBuffer.find("\r\n\r\n") + 4;
	if (_requestBuffer.size() - headersEndPos >= contentLength)
	{
		try
		{
			http->parserBody(_requestBuffer.substr(headersEndPos, contentLength)); // everything read
			return (true);
		}
		catch(const std::exception& e)
		{
			throw std::runtime_error(e.what());
		}
	}
	return (false); //keep reading
}

void Client::handleRequest(HTTPRequest *http)
{	
	http->logRequest(getCurrentTimestamp());

	// DO WE NEED TO PARSE FOR VALID HOSTNAMES???? and for $ variables?
	if (!http->validateRequest(_currentConfig, *this))
		return;
	handleCookies(http);
	try 
	{
		if (routeToCGI(http->resolvedFilePath))
		{
			int socketPair[2];
			if (socketpair(AF_UNIX, SOCK_STREAM, 0, socketPair) < 0)
			{
				prepareErrorResponse(500);
				return;
			}
			_cgi = new CGI(_serverLauncher, _currentConfig);
			_cgi->execute(http, socketPair);
			_cgiStartTime = std::time(NULL);
			_pendingRequest = http;
			_cgiPipeFD = socketPair[0];
			setCloexecFlag(_cgiPipeFD);

			_serverLauncher->registerCGIFD(_cgiPipeFD, _clientSocket);

			return;
		}
	}
	catch (const std::exception &e)
	{
		// std::cerr << e.what() << std::endl;
		//close client?
		prepareErrorResponse(500);
		return;
	}
	if (http->method == "GET")
		handleGET(http);
	else if (http->method == "POST")
		handlePOST(http);
	else if (http->method == "DELETE")
		handleDELETE(http);
	else
		prepareErrorResponse(405);

	resetState();
}

void Client::handleCookies(HTTPRequest *http)
{
	if (http->headers.find("Cookie") != http->headers.end())
		_cookies.parse(http->headers["Cookie"]);
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

void Client::handleCGIOutput(int fd)
{
	// std::cerr << "[CGI] Output on FD " << fd << " for client FD " << _clientSocket << std::endl;

	if (_cgiPipeFD == -1 || fd != _cgiPipeFD || !_cgi || !_pendingRequest)
		return;
	
	try {
		_cgi->handleCGIOutput(fd, _pendingRequest);
		if (_cgi->isComplete())
		{
			// std::cerr << "[CGI] complete" << std::endl;
			// std::cerr << "[DEBUG] Removing FD " << fd << ", current use: CGI\n";
			_cgiPipeFD = -1;
			
			prepareResponse(200, "text/html", _cgi->getOutput(), "", _cgi->getHeaders());

			_serverLauncher->removeCGIFD(fd);
			_serverLauncher->getEpoll().removeFD(fd); // cleanup epoll
			close(fd);
			delete _cgi;
			_cgi = NULL;
			delete _pendingRequest;
			_pendingRequest = NULL;
			_serverLauncher->getEpoll().modifyFD(_clientSocket, EPOLLOUT);
		}
	} catch (const std::exception &e) {
		// std::cerr << "[CGI] catch exception: " << fd << " e- " << e.what() << std::endl;
		if (_cgiPipeFD != -1) {
			_cgiPipeFD = -1;
			_serverLauncher->getEpoll().removeFD(fd);
			_serverLauncher->removeCGIFD(fd);
			close(fd);
		}
		if (_cgi) {
			delete _cgi;
			_cgi = NULL;
		}
		if (_pendingRequest) {
			delete _pendingRequest;
			_pendingRequest = NULL;
		}
		prepareErrorResponse(500);
		_serverLauncher->getEpoll().modifyFD(_clientSocket, EPOLLOUT);
	}
}

int Client::getSocket() {
	return	_clientSocket;
}
CGI* Client::getCGI() {
	return	_cgi;
}
time_t Client::getCGITime() {
	return	_cgiStartTime;
}
void Client::cleanupCGIState(int errorCode) {
	// std::cout << "Client::cleanupCGIState " << errorCode << std::endl;
	prepareErrorResponse(errorCode);
	writeResponse();
	if (_cgiPipeFD != -1)
	{
		close(_cgiPipeFD);
		_cgiPipeFD = -1;
	}
	if (_cgi)
	{
		delete _cgi;
		_cgi = NULL;
	}
	if (_pendingRequest)
	{
		delete _pendingRequest;
		_pendingRequest = NULL;
	}
}

void Client::handleGET(HTTPRequest	*http)
{
	std::string filePath = http->resolveFilePath(_currentConfig);
	// std::cout << "FILE PATH : " << filePath << std::endl;
	if (filePath == "403")
	{
		prepareErrorResponse(403);
		return;
	}

	struct stat pathStat;
	if (stat(filePath.c_str(), &pathStat) == 0 && S_ISDIR(pathStat.st_mode))
	{
		std::string directoryListing = HTTPResponse::directoryList(filePath, http->uri);
		prepareResponse(200, "text/html", directoryListing, "", "");
		return;
	}

	std::ifstream file(filePath.c_str(), std::ios::binary);
	if (!file)
	{
		prepareErrorResponse(404);
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

	if (http->headers.find("Content-Type") != http->headers.end() &&
		http->headers["Content-Type"].find("multipart/form-data") != std::string::npos)
	{
		std::string boundary = http->headers["Content-Type"].substr(http->headers["Content-Type"].find("boundary=") + 9);
		boundary.erase(boundary.find_last_not_of(" \t\r\n") + 1);

		std::string fullBoundary = "--" + boundary;
		std::string closingBoundary = fullBoundary + "--";

		size_t start = http->body.find(fullBoundary);
		if (start == std::string::npos)
		{
			prepareErrorResponse(400);//Starting boundary not found");
			return;
		}
		start += fullBoundary.length() + 2;

		size_t end = http->body.find("\r\n" + closingBoundary, start);
		if (end == std::string::npos)
			end = http->body.find("\n" + closingBoundary, start);
		if (end == std::string::npos)
			end = http->body.find(closingBoundary, start);
		if (end == std::string::npos)
		{
			prepareErrorResponse(400);//Bad Request: Ending boundary not found");
			return;
		}
		std::string part = http->body.substr(start, end - start);
		size_t headerEnd = part.find("\r\n\r\n");
		if (headerEnd == std::string::npos)
		{
			prepareErrorResponse(400);//Could not find headers in part");
			return;
		}
		headerEnd += 4;
		std::string fileContent = part.substr(headerEnd);

		size_t filenamePos = part.find("filename=\"");
		if (filenamePos == std::string::npos)
		{
			prepareErrorResponse(400);//Filename not found in Content-Disposition header");
			return;
		}
		filenamePos += 10;
		size_t filenameEnd = part.find("\"", filenamePos);
		if (filenameEnd == std::string::npos)
		{
			prepareErrorResponse(400);//Invalid filename in Content-Disposition header");
			return;
		}
		std::string filename = part.substr(filenamePos, filenameEnd - filenamePos);
		std::string filePath = root + "/" + filename;

		std::ofstream file(filePath.c_str(), std::ios::binary);
		if (!file)
		{
			prepareErrorResponse(500); //SERVER Error or BAD REQUEST?
			return;
		}
		file << fileContent;
		file.close();
	}
	else
	{
		std::string filename = http->headers["X-Filename"];
		filename.erase(std::remove(filename.begin(), filename.end(), '\r'), filename.end());
		filename.erase(std::remove(filename.begin(), filename.end(), '\n'), filename.end());
		filename.erase(std::remove(filename.begin(), filename.end(), '\''), filename.end());
		if (filename.empty())
		{
			prepareErrorResponse(400);//Filename header missing");
			return;
		}
		std::string filePath = root + "/" + filename;
		std::ofstream file(filePath.c_str(), std::ios::binary);
		if (!file)
		{
			prepareErrorResponse(500);
			return;
		}
		file << http->body;
		file.close();
	}
	prepareResponse(200, "text/plain", "File uploaded successfully\n", "", "");
}

void Client::handleDELETE(HTTPRequest *http)
{
	//[EVAL]try to delete something with and without permissions from config file and chmod000
	std::string root = http->resolveFilePath(_currentConfig);
	std::string filename = http->headers["X-Filename"];
	filename.erase(std::remove(filename.begin(), filename.end(), '\r'), filename.end());
	filename.erase(std::remove(filename.begin(), filename.end(), '\n'), filename.end());
	filename.erase(std::remove(filename.begin(), filename.end(), '\''), filename.end());
	if (filename.empty())
	{
		prepareErrorResponse(400);//Filename header missing");
		return;
	}
	std::string filePath = root + "/" + filename;
	if (std::remove(filePath.c_str()) == 0)
		prepareResponse(200, "text/plain", "File deleted successfully", "", "");
	else
		prepareErrorResponse(404);
}

void Client::prepareResponse(int statusCode, const std::string &contentType, const std::string &body, const std::string &redirect = "", const std::string &additionalHeaders = "")
{
	if (ErrorPage::isErrorStatusCode(statusCode))
	{
		prepareErrorResponse(statusCode);
		return;
	}
	HTTPResponse response;
	_responseBuffer = response.setResponse(statusCode, contentType, body, redirect, additionalHeaders);
}

std::string resolveErrorPage(int statusCode, const std::string& requestURI, const ConfigFileServer& config)
{
	const std::map<std::string, ConfigFileServerLocation> &locations = config.getLocations();
	size_t longestMatch = 0;
	const ConfigFileServerLocation *matchedLocation = NULL;
	std::string matchedPrefix;
	for (std::map<std::string, ConfigFileServerLocation>::const_iterator it = locations.begin(); it != locations.end(); ++it)
	{
		if (requestURI.find(it->first) == 0 && it->first.length() > longestMatch)
		{
			longestMatch = it->first.length();
			matchedLocation = &it->second;
			matchedPrefix = it->first;
		}
	}
	// 1. Check location-level error_page
	if (matchedLocation)
	{
		const std::map<int, std::string> &errorPages = matchedLocation->getErrorPages();
		std::map<int, std::string>::const_iterator errIt = errorPages.find(statusCode);
		if (errIt != errorPages.end())
		{
			std::string uri = errIt->second;
			std::stringstream ss;
			ss << errIt->first;
			if (matchedLocation->hasAlias())
			{
				std::string relativePath = "/" + ss.str() + uri;
				if (uri.find(matchedPrefix) == 0)
					relativePath = uri.substr(matchedPrefix.length());
				return (matchedLocation->getAlias() + relativePath);
			}
			else
				return (matchedLocation->getRoot() + "/" + ss.str() + uri);
		}
	}
	// 2. Fallback to server-level error pages
	const std::map<int, std::string> &serverErrorPages = config.getErrorPages();
	std::map<int, std::string>::const_iterator serverErr = serverErrorPages.find(statusCode);
	if (serverErr != serverErrorPages.end())
	{
		std::stringstream ss;
		ss << serverErr->first;
		std::string path = "/" + ss.str() + serverErr->second;
		if (!locations.empty())
		{
			// Use the first location root/alias as fallback
			const ConfigFileServerLocation &firstLoc = locations.begin()->second;
			if (firstLoc.hasAlias())
				return (firstLoc.getAlias() + path);
			else
				return (firstLoc.getRoot() + path);
		}
	}
	// Not found
	return "";
}
void Client::prepareErrorResponse(int statusCode)
{
	HTTPResponse response;
	std::string requestURI = "/"; // fallback
	size_t uriStart = _requestBuffer.find(" ");
	if (uriStart != std::string::npos)
	{
		size_t uriEnd = _requestBuffer.find(" ", uriStart + 1);
		if (uriEnd != std::string::npos)
			requestURI = _requestBuffer.substr(uriStart + 1, uriEnd - uriStart - 1);
	}
	std::string errorPagePath = resolveErrorPage(statusCode, requestURI, _currentConfig);
	if (!errorPagePath.empty())
	{
		std::ifstream file(errorPagePath.c_str(), std::ios::binary);
		if (file)
		{
			std::stringstream buffer;
			buffer << file.rdbuf();
			std::string body = buffer.str();
			response.setHeader("Connection", "close");
			_responseBuffer = response.setResponse(statusCode, "text/html", body, "", "");
			resetState();
			return;
		}
	}
	// Fallback to generated HTML
	std::string errorPage = ErrorPage::generate(statusCode);
	std::ifstream file(errorPage.c_str(), std::ios::binary);
	if (file)
	{
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string body = buffer.str();
		response.setHeader("Connection", "close");
		_responseBuffer = response.setResponse(statusCode, "text/html", body, "", "");
		ErrorPage::cleanup(errorPage);
	}
	else
	{
		std::string body = "<html><head><title>500 Internal Server Error</title></head>"
						   "<body><h1>500 Internal Server Error</h1>"
						   "<p>Something went wrong. Please try again later.</p></body></html>";
		response.setHeader("Connection", "close");
		_responseBuffer = response.setResponse(500, "text/html", body, "", "");
	}
	resetState();
}

bool Client::hasPendingData() const
{
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
			_keepAlive = false; // connection closed by client
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
			// std::cerr << " DEBUG - SET KEEPALIVE TRUE" << std::endl;
			_keepAlive = true;
		}
		else
			_keepAlive = false;
	}
}
