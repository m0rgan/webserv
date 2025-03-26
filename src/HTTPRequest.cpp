/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 14:58:18 by migumore          #+#    #+#             */
/*   Updated: 2025/03/23 13:52:59 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <HTTPRequest.hpp>

HTTPRequest::HTTPRequest(void) : _host(""), _port(80) {}

HTTPRequest::HTTPRequest(HTTPRequest const &src)
{
	*this = src;
};

HTTPRequest HTTPRequest::operator=(HTTPRequest const &rhs)
{
	if (this != &rhs)
	{
		this->request = rhs.request;
		this->_host = rhs._host;
		this->_port = rhs._port;
	}
	return (*this);
};

HTTPRequest::~HTTPRequest(void){};

void HTTPRequest::parserHeaders(const std::string &rawRequest)
{
	std::istringstream requestStream(rawRequest);
	std::string line;

	if (std::getline(requestStream, line))
	{
		std::istringstream lineStream(line);
		lineStream >> request.method >> request.uri >> request.httpVersion;
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
			request.headers[key] = value;
		}
	}
	try
	{
		if (request.headers.find("Content-Length") != request.headers.end())
			request.contentLength = parseContentLength(request.headers["Content-Length"]);
		else
			request.contentLength = 0;
	}
	catch (const std::invalid_argument &e)
	{
		throw std::runtime_error("400 Bad Request");
	}
	
	std::map<std::string, std::string>::const_iterator it = request.headers.find("Host");
	if (it != request.headers.end())
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
	std::string body;
	std::getline(requestStream, body, '\0');
	request.body = body;
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
		if (request.uri.find(it->first) == 0 && (matchedLocation.empty() || it->first.length() > matchedLocation.length()))
		{
			matchedLocation = it->first;
			resolvedRoot = it->second.getRoot();
			resolvedAlias = it->second.getAlias();
			resolvedIndexFiles = it->second.getIndexFiles();
		}
	}

	std::string requestUri = request.uri;
	if (!requestUri.empty() && requestUri[0] == '/')
		requestUri = requestUri.substr(1);

	std::string filePath;
	if (!resolvedAlias.empty())
	{
		filePath = resolvedAlias;
		std::string relativeUri = request.uri.substr(matchedLocation.length());
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

	if (request.method == "POST" || request.method == "DELETE")
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
			//what does nginx do when the is not index directive?
		}
		if (request.method == "GET")
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
		filePath = filePath.substr(0, filePath.size() - 1);

	std::string fileExtension;
	size_t dotPos = filePath.find_last_of('.');
	if (dotPos != std::string::npos)
	{
		fileExtension = filePath.substr(dotPos);
		std::string mimeType = getMimeType(fileExtension);
		if (!mimeType.empty())
			return (filePath);
	}

	return (filePath);
}

const std::string& HTTPRequest::getHost() const {return _host;}
int HTTPRequest::getPort() const {return _port;}

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
		if (!methods.empty() && std::find(methods.begin(), methods.end(), request.method) == methods.end())
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
	std::string body = "Redirecting to " + redirectUrl;
	client.prepareResponse(statusCode, "text/plain", body, redirectUrl, "");
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
		if (request.uri.find(it->first) == 0 && (bestLocation == NULL || it->first.length() > bestLocation->getURI().length()))
			bestLocation = &it->second;
	}
	return (bestLocation);
}

bool HTTPRequest::validateRequest(const ConfigFileServer &config, Client &client)
{
	
	if (request.method != "GET" && request.method != "POST" && request.method != "DELETE")
		return (client.prepareErrorResponse(400), (false));
	const ConfigFileServerLocation *matchedLocation = matchLocation(config);

	if (locationReturn(matchedLocation, client))
		return (false);
	if (serverReturn(config, client))
		return (false);
	if (matchedLocation && !isMethodAllowed(matchedLocation))
		return (client.prepareErrorResponse(405), (false));
	if (request.contentLength > config.getMaxBodySize())
		return (client.prepareErrorResponse(413), (false));
	resolvedFilePath = resolveFilePath(config);
	if (resolvedFilePath.empty())
		return (client.prepareErrorResponse(404), (false));
	return (true);
}

void HTTPRequest::logRequest(const std::string timestamp) const
{
	std::string color;

	if (request.method == "GET")
		color = GREEN;
	else if (request.method == "POST")
		color = MAGENTA;
	else if (request.method == "DELETE")
		color = ORANGE;
	std::cout << color << "[" << timestamp << "] ";
	std::cout << request.httpVersion << " " << request.method << " " << request.uri << std::endl;
	// for (std::map<std::string, std::string>::const_iterator it = request.headers.begin(); it != request.headers.end(); ++it)
	// 	std::cout << it->first << ": " << it->second << std::endl;
	// if (!request.body.empty())
	// 	std::cout << std::endl << request.body << std::endl;
	std::cout << RESET;
}
