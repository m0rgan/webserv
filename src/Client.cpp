/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 17:00:36 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/04/01 19:11:23 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(void) : _sessionManager(*(new SessionManagement())) {}

Client::Client(int socket, const ConfigFileServer &config, SessionManagement &sessionManager) : _clientSocket(socket), _bytesSent(0), _currentConfig(config), _sessionManager(sessionManager) { setCloexecFlag(_clientSocket); }

Client::Client(Client const &src) : _clientSocket(src._clientSocket), _requestBuffer(src._requestBuffer), _responseBuffer(src._responseBuffer), _bytesSent(src._bytesSent), _currentConfig(src._currentConfig), _sessionManager(src._sessionManager) {}

Client &Client::operator=(Client const &rhs)
{
	if (this != &rhs)
	{
		this->_clientSocket = rhs._clientSocket;
		this->_requestBuffer = rhs._requestBuffer;
		this->_responseBuffer = rhs._responseBuffer;
		this->_bytesSent = rhs._bytesSent;
		this->_currentConfig = rhs._currentConfig;
		this->_keepAlive = rhs._keepAlive;
	}
	return (*this);
}

Client::~Client() {}

const ConfigFileServer& Client::getConfigFileServer() const { return _currentConfig; }
void Client::setConfigFileServer(const ConfigFileServer &config) { _currentConfig = config; }
bool Client::keepAlive() const { return (_keepAlive); }

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
					prepareErrorResponse(400);
					throw std::runtime_error("400 Bad Request");
				}
				if (http.contentLength == 0 && http.headers.find("Transfer-Encoding") == http.headers.end())
					break;
			}
			try
			{
				if (headersRead && (lengthData(http) || chunkedData(http)))
					break;
			}
			catch(const std::runtime_error &e)
			{
				prepareErrorResponse(400);
				throw std::runtime_error("400 Bad Request");
			}
		}
		else if (bytesRead == 0)
		{
			_keepAlive = false;
			break;
		}
		else
			break;
	}
	if (http.headers.find("Connection") != http.headers.end() && http.headers["Connection"] == "close")
		_keepAlive = false;
	return (http);
}

bool Client::chunkedData(HTTPRequest &http)
{
	if (http.headers.find("Transfer-Encoding") != http.headers.end() &&
		http.headers["Transfer-Encoding"] == "chunked")
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
		http.body = body;
		if (http.method == "POST" && http.body.empty())
		{
			throw std::runtime_error("400 Bad Request");
		}
		return (true); // everything read
	}
	return (false);
}

bool Client::lengthData(HTTPRequest &http)
{
	if (http.contentLength == 0)
		return (true);

	size_t contentLength = http.contentLength;
	size_t headersEndPos = _requestBuffer.find("\r\n\r\n") + 4;
	if (_requestBuffer.size() - headersEndPos >= contentLength)
	{
		try
		{
			http.parserBody(_requestBuffer.substr(headersEndPos, contentLength)); // everything read
			return (true);
		}
		catch(const std::exception& e)
		{
			throw std::runtime_error(e.what());
		}
	}
	return (false); //keep reading
}

void Client::handleRequest(HTTPRequest &http, ServerLauncher* server)
{	
	http.logRequest(getCurrentTimestamp());

	// DO WE NEED TO PARSE FOR VALID HOSTNAMES???? and for $ variables?
	
	if (!http.validateRequest(_currentConfig, *this))
		return;
	
	handleCookies(http);
	try 
	{
		if (routeToCGI(http.resolvedFilePath))
		{
			CGI cgi(server, _currentConfig);
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
		// std::cerr << e.what() << std::endl;
		//close client?
		prepareErrorResponse(500);
		return;
	}
	if (http.method == "GET")
		handleGET(&http);
	else if (http.method == "POST")
		handlePOST(&http);
	else if (http.method == "DELETE")
		handleDELETE(&http);
	else
		prepareErrorResponse(405);

	resetState();
}

void Client::handleCookies(HTTPRequest &http)
{
	if (http.headers.find("Cookie") != http.headers.end())
		_cookies.parse(http.headers["Cookie"]);
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
			std::cerr << "SS IS BEING PRINTED ====" << ss.str() << std::endl;
			if (matchedLocation->hasAlias())  // You’ll need to implement this if not already
			{
				std::string relativePath = "/" + ss.str() + uri;
				std::cerr << "realtive path ====" << relativePath << std::endl;
				if (uri.find(matchedPrefix) == 0)
					relativePath = uri.substr(matchedPrefix.length());
				std::cerr << "first matCHED	location ====" << matchedLocation->getAlias() << relativePath << std::endl;
				return (matchedLocation->getAlias() + relativePath);
			}
			else
			{
				std::cerr << "second matCHED	location ====" << matchedLocation->getRoot() << "/" << ss.str() << uri << std::endl;
				return (matchedLocation->getRoot() + "/" + ss.str() + uri);
			}
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
	return "HOLA";
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
		std::cerr << "ERROR PAGE PATH NOT EMPY ====" << std::endl;
		std::ifstream file(errorPagePath.c_str(), std::ios::binary);
		if (file)
		{
			std::cerr << "FILE NOY EMPTY ====" << std::endl;
			std::stringstream buffer;
			buffer << file.rdbuf();
			std::string body = buffer.str();
			response.setHeader("Connection", "close");
			_responseBuffer = response.setResponse(statusCode, "text/html", body, "", "");
			resetState();
			return;
		}
		else
		{
			std::cerr << "[ERROR] Failed to open error page file: " << errorPagePath << std::endl;
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


// void Client::prepareErrorResponse(int statusCode)
// {
// 	HTTPResponse response;
// 	const std::map<int, std::string> &errorPages = _currentConfig.getErrorPages();
// 	std::map<int, std::string>::const_iterator it = errorPages.find(statusCode);

// 	if (it != errorPages.end())
// 	{
// 		std::string errorPagePath;
// 		std::stringstream ss;
// 		ss << it->first;
// 		const std::map<std::string, ConfigFileServerLocation> &locations = _currentConfig.getLocations();
// 		for (std::map<std::string, ConfigFileServerLocation>::const_iterator ite = locations.begin(); ite != locations.end(); ++ite)
// 		{
// 			errorPagePath = ite->second.getRoot() + "/" + ss.str() + it->second;
// 			std::cerr << errorPagePath << std::endl;
// 			std::ifstream file(errorPagePath.c_str(), std::ios::binary);
// 				if (file)
// 					break;	
// 		}
// 		std::ifstream file(errorPagePath.c_str(), std::ios::binary);
// 		if (file)
// 		{
// 			std::stringstream buffer;
// 			buffer << file.rdbuf();
// 			std::string body = buffer.str();
// 			response.setHeader("Connection", "close");
// 			_responseBuffer = response.setResponse(statusCode, "text/html", body, "", "");
// 			resetState();
// 			return;
// 		}
// 		else
// 			std::cerr << "[ERROR] Failed to open custom error page: " << errorPagePath << std::endl;
// 	}

// 	std::string errorPage = ErrorPage::generate(statusCode);
// 	std::ifstream file(errorPage.c_str(), std::ios::binary);
// 	if (file)
// 	{
// 		std::stringstream buffer;
// 		buffer << file.rdbuf();
// 		std::string body = buffer.str();
// 		response.setHeader("Connection", "close");
// 		_responseBuffer = response.setResponse(statusCode, "text/html", body, "", "");
// 		ErrorPage::cleanup(errorPage);
// 	}
// 	else
// 	{
// 		std::string body = "<html><head><title>500 Internal Server Error</title></head>"
// 						"<body><h1>500 Internal Server Error</h1>"
// 						"<p>Something went wrong. Please try again later.</p></body></html>";
// 		response.setHeader("Connection", "close");
// 		_responseBuffer = response.setResponse(500, "text/html", body, "", "");
// 	}
// 	resetState();
// }
// Connected to localhost (::1) port 443 this is false because secure connection self signed certs dont work

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
