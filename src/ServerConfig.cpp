/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 13:01:34 by migumore          #+#    #+#             */
/*   Updated: 2025/02/05 18:28:46 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ServerConfig.hpp>
#include <iostream>

#include <sys/socket.h>
#include <sstream>
#include <fstream>

ServerConfig::ServerConfig() : autoIndex(false), maxBodySize(1048576), clientTimeout(30)
{
	ports.clear();
}

ServerConfig::ServerConfig(const ServerConfig &other)
{
	*this = other;
}

ServerConfig &ServerConfig::operator=(const ServerConfig &other)
{
	if (this != &other)
	{
		ports = other.ports;
		serverName = other.serverName;
		errorPages = other.errorPages;
		locations = other.locations;
		methods = other.methods;
		cgiExtension = other.cgiExtension;
		indexFiles = other.indexFiles;
		autoIndex = other.autoIndex;
		maxBodySize = other.maxBodySize;
		clientTimeout = other.clientTimeout;
		logAccessFile = other.logAccessFile;
		logErrorFile = other.logErrorFile;
		redirects = other.redirects;
		upstreamServers = other.upstreamServers;
		allowList = other.allowList;
		denyList = other.denyList;
		mimeTypes = other.mimeTypes;
	}
	return *this;
}

ServerConfig::~ServerConfig() {}

const std::vector<int> &ServerConfig::getPorts() const { return ports; }
const std::string &ServerConfig::getServerName() const { return serverName; }
const std::map<int, std::string> &ServerConfig::getErrorPages() const { return errorPages; }
const std::map<std::string, std::string> &ServerConfig::getLocations() const { return locations; }
const std::map<std::string, std::vector<std::string> > &ServerConfig::getMethods() const { return methods; }
const std::string &ServerConfig::getCgiExtension() const { return cgiExtension; }
const std::vector<std::string> &ServerConfig::getIndexFiles() const { return indexFiles; }
bool ServerConfig::isAutoIndex() const { return autoIndex; }
int ServerConfig::getMaxBodySize() const { return maxBodySize; }
int ServerConfig::getClientTimeout() const { return clientTimeout; }
const std::string &ServerConfig::getLogAccessFile() const { return logAccessFile; }
const std::string &ServerConfig::getLogErrorFile() const { return logErrorFile; }
const std::map<std::string, std::string> &ServerConfig::getRedirects() const { return redirects; }
const std::vector<std::string> &ServerConfig::getUpstreamServers() const { return upstreamServers; }
const std::vector<std::string> &ServerConfig::getAllowList() const { return allowList; }
const std::vector<std::string> &ServerConfig::getDenyList() const { return denyList; }
const std::map<std::string, std::string> &ServerConfig::getMimeTypes() const { return mimeTypes; }

void ServerConfig::addPort(int port) { ports.push_back(port); }
void ServerConfig::setServerName(const std::string &name) { serverName = name; }
void ServerConfig::addErrorPage(int errorCode, const std::string &pagePath) { errorPages[errorCode] = pagePath; }
void ServerConfig::addLocation(const std::string &uri, const std::string &path) { locations[uri] = path; }
void ServerConfig::addMethod(const std::string &uri, const std::string &method) { methods[uri].push_back(method); }
void ServerConfig::setCgiExtension(const std::string &ext) { cgiExtension = ext; }
void ServerConfig::addIndexFile(const std::string &file) { indexFiles.push_back(file); }
void ServerConfig::setAutoIndex(bool enabled) { autoIndex = enabled; }
void ServerConfig::setMaxBodySize(int size) { maxBodySize = size; }
void ServerConfig::setClientTimeout(int timeout) { clientTimeout = timeout; }
void ServerConfig::setLogAccessFile(const std::string &path) { logAccessFile = path; }
void ServerConfig::setLogErrorFile(const std::string &path) { logErrorFile = path; }
void ServerConfig::addRedirect(const std::string &from, const std::string &to) { redirects[from] = to; }
void ServerConfig::addUpstreamServer(const std::string &backend) { upstreamServers.push_back(backend); }
void ServerConfig::addAllowIP(const std::string &ip) { allowList.push_back(ip); }
void ServerConfig::addDenyIP(const std::string &ip) { denyList.push_back(ip); }
void ServerConfig::addMimeType(const std::string &extension, const std::string &mimeType) { mimeTypes[extension] = mimeType; }

void ServerConfig::printConfig() const
{
	std::cout << "· Server Name: " << serverName << std::endl;
	std::cout << "· Ports: ";
	for (size_t i = 0; i < ports.size(); i++)
	{
		std::cout << ports[i] << " ";
	}
	std::cout << std::endl;

	std::cout << "· Error Pages:" << std::endl;
	for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
	{
		std::cout << "  " << it->first << " -> " << it->second << std::endl;
	}

	std::cout << "· Index Files: ";
	for (size_t i = 0; i < indexFiles.size(); i++)
	{
		std::cout << indexFiles[i] << " ";
	}
	std::cout << std::endl;

	std::cout << "· Auto-Index: " << (autoIndex ? "on" : "off") << std::endl;
	std::cout << "· Max Body Size: " << maxBodySize << " bytes" << std::endl;
	std::cout << "· Client Timeout: " << clientTimeout << " seconds" << std::endl;
	std::cout << "· Logging:\n\t- Access: " << logAccessFile << "\n\t- Error: " << logErrorFile << std::endl;

	std::cout << "· Redirects:" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = redirects.begin(); it != redirects.end(); ++it)
	{
		std::cout << "  " << it->first << " -> " << it->second << std::endl;
	}

	std::cout << "· Allowed MIME Types:" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = mimeTypes.begin(); it != mimeTypes.end(); ++it)
	{
		std::cout << "  " << it->first << " -> " << it->second << std::endl;
	}
}

void ServerConfig::sendResponse(int clientSocket, const std::string &status, const std::string &contentType, const std::string &body) const
{
	std::ostringstream response; //no idea if this is compliant
	response << "HTTP/1.1 " << status << "\r\n";
	response << "Content-Type: " << contentType << "\r\n";
	response << "Content-Length: " << body.size() << "\r\n";
	response << "Connection: close\r\n";
	response << "\r\n";
	response << body;

	std::string responseStr = response.str();
	send(clientSocket, responseStr.c_str(), responseStr.size(), 0);

std::cout << "Sent response: " << responseStr << std::endl;
}

void ServerConfig::handleRequest(int clientSocket, const std::string &request) const
{
	std::istringstream requestStream(request);
	std::string method;
	std::string path;
	requestStream >> method >> path;

	if (method == "GET")
	{
		if (path == "/")
			path = "/index.html"; // example of static ?? get from configFile??
		std::ifstream file("." + path);
		if (file)
		{
			std::stringstream buffer;
			buffer << file.rdbuf();
			sendResponse(clientSocket, "200 OK", "text/html", buffer.str());
		}
		else
			sendResponse(clientSocket, "404 Not Found", "text/plain", "404 Not Found");
	}
	else // need POST DELETE and method not supported
		sendResponse(clientSocket, "405 Method Not Allowed", "text/plain", "405 Method Not Allowed");
}
