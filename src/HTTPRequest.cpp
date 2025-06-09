/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 14:58:18 by migumore          #+#    #+#             */
/*   Updated: 2025/05/09 13:31:34 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <HTTPRequest.hpp>

HTTPRequest::HTTPRequest(void) : _host(""), _port(80), contentLength(0) {}

HTTPRequest::HTTPRequest(HTTPRequest const &src)
{
	*this = src;
}

HTTPRequest HTTPRequest::operator=(HTTPRequest const &rhs)
{
	if (this != &rhs)
	{
		this->_host = rhs._host;
		this->_port = rhs._port;
		this->method = rhs.method;
		this->uri = rhs.uri;
		this->httpVersion = rhs.httpVersion;
		this->headers = rhs.headers;
		this->body = rhs.body;
		this->contentLength = rhs.contentLength;
		this->resolvedFilePath = rhs.resolvedFilePath;
	}
	return (*this);
}

HTTPRequest::~HTTPRequest(void)
{
	headers.clear();
    // Fuerza que la memoria asignada internamente se libere.
    std::map<std::string, std::string>().swap(headers);
    
    body.clear();
    std::string().swap(body);
}

void HTTPRequest::parserHeaders(const std::string &rawRequest)
{
	std::istringstream requestStream(rawRequest);
	std::string line;

	if (std::getline(requestStream, line))
	{
		if (line.empty()) {
			throw std::runtime_error("400 Bad Request");
		} else {
			std::istringstream lineStream(line);
			if (!(lineStream >> method >> uri >> httpVersion)) {
				throw std::runtime_error("400 Bad Request");
			}
		}
		if ((method != "GET" && method != "POST" && method != "DELETE")
			|| httpVersion != "HTTP/1.1"
			|| (uri.empty() || uri[0] != '/'))
			throw std::runtime_error("400 Bad Request");
	}
	while (std::getline(requestStream, line) && line != "\r")
	{
		std::size_t colonPos = line.find(':');
		if (colonPos != std::string::npos)
		{
			std::string key = line.substr(0, colonPos);
			std::string value = line.substr(colonPos + 1);
			while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
				value.erase(0, 1);
			headers.insert(std::make_pair(key, value));
			// headers[key] = value;
		}
	}
	try
	{
		if (headers.find("Content-Length") != headers.end())
			contentLength = parseContentLength(headers["Content-Length"]);
		else
			contentLength = 0;
	}
	catch (const std::invalid_argument &e)
	{
		throw std::runtime_error("400 Bad Request");
	}
	
	std::map<std::string, std::string>::const_iterator it = headers.find("Host");
	if (it != headers.end())
	{
		_host = it->second;
		size_t colonPos = _host.find(':');
		if (colonPos != std::string::npos)
		{
			_port = static_cast<int>(stringTUL(_host.substr(colonPos + 1)));
			_host = _host.substr(0, colonPos);
		}
		else
			_port = 80;
	}
}

void HTTPRequest::parserBody(const std::string &rawRequest)
{
	std::istringstream requestStream(rawRequest);
	std::string bodyStr;
	std::getline(requestStream, bodyStr, '\0');
	body = bodyStr;
	if (method == "POST" && body.empty())
	{
		throw std::runtime_error("400 Bad Request");
	}
	
}

std::string HTTPRequest::resolveFilePath(const ConfigFileServer &config) const
{
	std::string matchedLocation;
	std::string resolvedRoot = config.getRoot();
	std::string resolvedAlias;
	std::vector<std::string> resolvedIndexFiles = config.getIndexFiles();
	const std::map<std::string, ConfigFileServerLocation> &locations = config.getLocations();

	for (std::map<std::string, ConfigFileServerLocation>::const_iterator it = locations.begin(); it != locations.end(); ++it)
	{
		if (uri.find(it->first) == 0 && (matchedLocation.empty() || it->first.length() > matchedLocation.length()))
		{
			matchedLocation = it->first;
			resolvedRoot = it->second.getRoot();
			resolvedAlias = it->second.getAlias();
			resolvedIndexFiles = it->second.getIndexFiles();
		}
	}

	std::string requestUri = uri;
	if (!requestUri.empty() && requestUri[0] == '/')
		requestUri = requestUri.substr(1);

	std::string filePath;
	if (!resolvedAlias.empty())
	{
		filePath = resolvedAlias;
		std::string relativeUri = uri.substr(matchedLocation.length());
		if (!relativeUri.empty() && relativeUri[0] == '/')
			relativeUri = relativeUri.substr(1);
		if (!filePath.empty() && filePath[filePath.size() - 1] != '/')
			filePath += "/";
		filePath += relativeUri;
	}
	else
	{
		filePath = resolvedRoot;
		if (!resolvedRoot.empty() && resolvedRoot[resolvedRoot.size() - 1] != '/')
			filePath += "/";
		if (!requestUri.empty() && requestUri[0] == '/')
			requestUri = requestUri.substr(1);
		filePath += requestUri;
	}

	if (method == "POST" || method == "DELETE")
		return (filePath);

	struct stat pathStat;
	if (stat(filePath.c_str(), &pathStat) == 0 && S_ISDIR(pathStat.st_mode))
	{
		for (size_t i = 0; i < resolvedIndexFiles.size(); ++i)
		{
			std::string indexPath = filePath;
			if (filePath[filePath.size() - 1] != '/')
				indexPath += "/";
			indexPath += resolvedIndexFiles[i];
			std::ifstream file(indexPath.c_str());
			if (file.good())
				return (indexPath);
			//what does nginx do when there is not index directive?
		}
		if (method == "GET")
		{
			if (config.getAutoIndex())
				return (filePath + "/");
			else
				return ("403");
		}
	}

	std::ifstream fileCheck(filePath.c_str());
	if (!fileCheck.good())
		return ("");

	if (!filePath.empty() && filePath[filePath.size() - 1] == '/')
		filePath = filePath.substr(0, filePath.size() - 1); //probar cpn nginx un  request de archivo con / al final

	return (filePath);
}

const std::string& HTTPRequest::getHost() const {return _host;}
int HTTPRequest::getPort() const {return _port;}
// content length esta en cero causando que no funcione y max body en / esta reescribiendo el del server directive AQUIIIII
size_t HTTPRequest::parseContentLength(std::string contentLengthStr)
{
	if (contentLengthStr.empty())
		return (0);

	std::stringstream ss(contentLengthStr);
	size_t value;
	char unit = '\0';

	ss >> value;
	if (!ss.eof())
		ss >> unit;
	unit = std::toupper(unit);

	switch (unit)
	{
		case 'K': value *= 1024; break;
		case 'M': value *= 1024 * 1024; break;
		case 'G': value *= 1024 * 1024 * 1024; break;
		case '\0': break;
		default: 
			throw std::invalid_argument("Invalid Content-Length unit: " + std::string(1, unit));
	}
	return (value);
}

bool HTTPRequest::isMethodAllowed(const ConfigFileServerLocation *location) const
{
	if (location)
	{
		const std::vector<std::string> &methods = location->getAllowedMethods();
		if (!methods.empty() && std::find(methods.begin(), methods.end(), method) == methods.end())
			return (false);
	}
	return (true);
}

bool HTTPRequest::handleReturnDirective(int statusCode, const std::string &redirectUrl, Client &client) const
{
	if (ErrorPage::isErrorStatusCode(statusCode))
	{
		client.prepareErrorResponse(statusCode);
		return (true);
	}
	std::stringstream ss;
	ss << "<html>\r\n"
	<< "<head><title>" << statusCode << " Moved Permanently</title></head>\r\n"
	<< "<body>\r\n"
	<< "<center><h1>" << statusCode << " Moved Permanently</h1></center>\r\n"
	<< "<hr><center>webserv</center>\r\n"
	<< "</body>\r\n"
	<< "</html>\r\n";
	std::string headers = "Location: " + redirectUrl + "\r\n";
	client.prepareResponse(statusCode, "text/html", ss.str(), redirectUrl, headers);
	return (true);
}

bool HTTPRequest::serverReturn(const ConfigFileServer &config, Client &client) const
{
	if (config.hasReturnDirective())
	{
		int statusCode = config.getReturnStatusCode();
		std::string redirect = config.getReturnUrl();
		return handleReturnDirective(statusCode, redirect, client);
	}
	return (false);
}

bool HTTPRequest::locationReturn(const ConfigFileServerLocation *location, Client &client) const
{
	if (location && location->hasReturnDirective())
	{
		int statusCode = location->getReturnStatusCode();
		std::string redirect = location->getReturnUrl();
		return handleReturnDirective(statusCode, redirect, client);
	}
	return (false);
}

const ConfigFileServerLocation* HTTPRequest::matchLocation(const ConfigFileServer &config) const
{
	const ConfigFileServerLocation *bestLocation = NULL;
	const std::map<std::string, ConfigFileServerLocation> &locations = config.getLocations();
	for (std::map<std::string, ConfigFileServerLocation>::const_iterator it = locations.begin(); it != locations.end(); ++it)
	{
		if (uri.find(it->first) == 0 && (bestLocation == NULL || it->first.length() > bestLocation->getURI().length()))
			bestLocation = &it->second;
	}
	return (bestLocation);
}

bool HTTPRequest::validateRequest(const ConfigFileServer &config, Client &client)
{
	const ConfigFileServerLocation *matchedLocation = matchLocation(config);

	if (locationReturn(matchedLocation, client))
		return (false);
	if (serverReturn(config, client))
		return (false);
	if (matchedLocation && !isMethodAllowed(matchedLocation))
		return (client.prepareErrorResponse(405), (false));
	// std::cerr << "HTTPRequest::validateRequest " << contentLength << "-"<<  config.getRoot() <<"----------"<<  matchedLocation->getURI() << "-"<<  matchedLocation->getMaxBodySize() << std::endl;
	if (contentLength > matchedLocation->getMaxBodySize())
		return (client.prepareErrorResponse(413), (false));
	resolvedFilePath = resolveFilePath(config);
	if (resolvedFilePath.empty())
		return (client.prepareErrorResponse(404), (false));
	return (true);
}

void HTTPRequest::logRequest(const std::string timestamp) const
{
	std::string color;

	if (method == "GET")
		color = GREEN;
	else if (method == "POST")
		color = MAGENTA;
	else if (method == "DELETE")
		color = ORANGE;
	std::cout << color << "[" << timestamp << "] ";
	std::cout << httpVersion << " " << method << " " << uri;
	std::cout << RESET << std::endl;

	// for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
	// 	std::cout << it->first << ": " << it->second << std::endl;
	// if (!body.empty())
	// 	std::cout << std::endl << body << std::endl;
	// std::cout << RESET;
}
