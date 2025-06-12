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
				return (false);
			std::string chunkSizeStr = _requestBuffer.substr(headerEnd, chunkEnd - headerEnd);
			size_t chunkSize = stringTUL(chunkSizeStr);
			headerEnd = chunkEnd + 2;
			if (chunkSize == 0)
				break;
			if (headerEnd + chunkSize > _requestBuffer.size())
				return (false);
			body.append(_requestBuffer.substr(headerEnd, chunkSize));
			headerEnd += chunkSize + 2;
		}
		http->body = body;
		if (http->method == "POST" && http->body.empty())
		{
			throw std::runtime_error("400 Bad Request");
		}
		return (true);
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
			http->body = _requestBuffer.substr(headersEndPos, contentLength);
			if (http->method == "POST" && http->body.empty())
			{
				throw std::runtime_error("400 Bad Request");
			}
			return (true);
		}
		catch(const std::exception& e)
		{
			throw std::runtime_error(e.what());
		}
	}
	return (false);
}

void Client::handleRequest(HTTPRequest *http)
{	
	http->logRequest(getCurrentTimestamp());

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

	if (!sessionID.empty() && !_sessionManager.sessionExists(sessionID))
	{
		_sessionManager.createSession(sessionID);
	}
	else if (sessionID.empty()) 
	{
		sessionID = _sessionManager.createSession("");
		_cookies.setCookie("SESSIONID", sessionID + "; Path=/; HttpOnly");
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
	if (_cgiPipeFD == -1 || fd != _cgiPipeFD || !_cgi || !_pendingRequest)
		return;
	
	try {
		_cgi->readCGIOutput(fd);
		if (_cgi->isComplete())
		{
			_cgiPipeFD = -1;
			
			prepareResponse(200, "text/html", _cgi->getOutput(), "", _cgi->getHeaders());

			_serverLauncher->removeCGIFD(fd);
			_serverLauncher->getEpoll().removeFD(fd);
			close(fd);
			delete _cgi;
			_cgi = NULL;
			delete _pendingRequest;
			_pendingRequest = NULL;
			_serverLauncher->getEpoll().modifyFD(_clientSocket, EPOLLOUT);
		}
	} catch (const std::exception &e) {
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

int Client::getSocket() { return _clientSocket; }
CGI* Client::getCGI() { return _cgi; }
time_t Client::getCGITime() { return _cgiStartTime; }

void Client::cleanupCGIState(int errorCode) {

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
			prepareErrorResponse(400);
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
			prepareErrorResponse(400);
			return;
		}
		std::string part = http->body.substr(start, end - start);
		size_t headerEnd = part.find("\r\n\r\n");
		if (headerEnd == std::string::npos)
		{
			prepareErrorResponse(400);
			return;
		}
		headerEnd += 4;
		std::string fileContent = part.substr(headerEnd);

		size_t filenamePos = part.find("filename=\"");
		if (filenamePos == std::string::npos)
		{
			prepareErrorResponse(400);
			return;
		}
		filenamePos += 10;
		size_t filenameEnd = part.find("\"", filenamePos);
		if (filenameEnd == std::string::npos)
		{
			prepareErrorResponse(400);
			return;
		}
		std::string filename = part.substr(filenamePos, filenameEnd - filenamePos);
		std::string filePath = root + "/" + filename;

		std::ofstream file(filePath.c_str(), std::ios::binary);
		if (!file)
		{
			prepareErrorResponse(500);
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
			prepareErrorResponse(400);
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
    std::string filePath = http->resolveFilePath(_currentConfig);

    if (filePath.empty()) {
        prepareErrorResponse(400);
        return;
    }

    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        if (errno == ENOENT) {
            prepareErrorResponse(404);
        } else if (errno == EACCES) {
            prepareErrorResponse(403);
        } else {
            prepareErrorResponse(500);
        }
        return;
    }

    if (!S_ISREG(fileStat.st_mode)) {
        if (S_ISDIR(fileStat.st_mode)) {
            prepareErrorResponse(409);
        } else {
            prepareErrorResponse(400);
        }
        return;
    }

    if (std::remove(filePath.c_str()) == 0) {
        prepareResponse(200, "text/plain", "File deleted successfully", "", "");
    } else {
        if (errno == EACCES || errno == EPERM) {
            prepareErrorResponse(403);
        } else if (errno == ENOENT) {
            prepareErrorResponse(404);
        } else if (errno == EISDIR) {
            prepareErrorResponse(409);
        } else {
            prepareErrorResponse(500);
        }
    }
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

	const std::map<int, std::string> &serverErrorPages = config.getErrorPages();
	std::map<int, std::string>::const_iterator serverErr = serverErrorPages.find(statusCode);
	if (serverErr != serverErrorPages.end())
	{
		std::stringstream ss;
		ss << serverErr->first;
		std::string path = "/" + ss.str() + serverErr->second;
		if (!locations.empty())
		{
			const ConfigFileServerLocation &firstLoc = locations.begin()->second;
			if (firstLoc.hasAlias())
				return (firstLoc.getAlias() + path);
			else
				return (firstLoc.getRoot() + path);
		}
	}
	return "";
}
void Client::prepareErrorResponse(int statusCode)
{
	HTTPResponse response;
	std::string requestURI = "/";
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
		
		if (written == -1)
			continue;
		if (written == 0)
		{
			_keepAlive = false;
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
			_keepAlive = false;
	}
}
