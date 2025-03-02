/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfigLocation.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 15:14:37 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/25 15:14:37 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfigLocation.hpp"

ServerConfigLocation::ServerConfigLocation() : autoIndex(false), maxBodySize(0) {}
ServerConfigLocation::ServerConfigLocation(const std::string &uri) : uri(uri), autoIndex(false), maxBodySize(0) {}
ServerConfigLocation::ServerConfigLocation(const ServerConfigLocation &src) {*this = src;} //fix
ServerConfigLocation &ServerConfigLocation::operator=(const ServerConfigLocation &rhs)
{
	if (this != &rhs)
	{
		this->uri = rhs.uri;
		this->root = rhs.root;
		this->indexFiles = rhs.indexFiles;
		this->allowedMethods = rhs.allowedMethods;
		this->autoIndex = rhs.autoIndex;
		this->cgiHandlers = rhs.cgiHandlers;
		this->errorPages = rhs.errorPages;
		this->headers = rhs.headers;
		this->redirection = rhs.redirection;
		this->maxBodySize = rhs.maxBodySize;
		this->nestedLocations = rhs.nestedLocations;
		//some are missing
	}
	return (*this);
}

ServerConfigLocation::~ServerConfigLocation() {}

void ServerConfigLocation::setURI(const std::string &uri) { this->uri = uri; }
const std::string &ServerConfigLocation::getURI() const { return this->uri; }
void ServerConfigLocation::setRoot(const std::string &root) { this->root = root; }
const std::string &ServerConfigLocation::getRoot() const { return this->root; }
void ServerConfigLocation::addIndexFile(const std::string &file) { indexFiles.push_back(file); }
const std::vector<std::string> &ServerConfigLocation::getIndexFiles() const { return indexFiles; }
void ServerConfigLocation::setAutoIndex(bool enabled) { autoIndex = enabled; }
bool ServerConfigLocation::getAutoIndex() const { return autoIndex; }
void ServerConfigLocation::setProxyPass(const std::string &proxyPass) { this->proxyPass = proxyPass; }
const std::string &ServerConfigLocation::getProxyPass() const { return proxyPass; }
void ServerConfigLocation::addAllowedMethod(const std::string &method) { allowedMethods.push_back(method); }
const std::vector<std::string> &ServerConfigLocation::getAllowedMethods() const { return allowedMethods; }
void ServerConfigLocation::addCgiHandler(const std::string &extension, const std::string &handler) { cgiHandlers[extension] = handler; }
const std::map<std::string, std::string> &ServerConfigLocation::getCgiHandlers() const { return cgiHandlers; }
void ServerConfigLocation::addErrorPage(int errorCode, const std::string &pagePath) { errorPages[errorCode] = pagePath; }
const std::map<int, std::string> &ServerConfigLocation::getErrorPages() const { return errorPages; }
void ServerConfigLocation::addHeader(const std::string &key, const std::string &value) { headers[key] = value; }
const std::map<std::string, std::string> &ServerConfigLocation::getHeaders() const { return headers; }
void ServerConfigLocation::setRedirection(const std::string &to) { redirection = to; }
const std::string &ServerConfigLocation::getRedirection() const { return redirection; }
void ServerConfigLocation::setMaxBodySize(int size) { maxBodySize = size; }
int ServerConfigLocation::getMaxBodySize() const { return maxBodySize; }
void ServerConfigLocation::addNestedLocation(const std::string &uri, const ServerConfigLocation &location){nestedLocations[uri] = location;}
const std::map<std::string, ServerConfigLocation> &ServerConfigLocation::getNestedLocations() const {return nestedLocations; }

bool ServerConfigLocation::matchesURI(const std::string &requestURI) const
{
	if (uri.empty())
		return (false);
	// check for simple prefix match?
	return (requestURI.find(uri) == 0);
}

void ServerConfigLocation::printLocationConfig() const
{
	std::cout << "  Location: " << uri << std::endl;
	std::cout << "  Root: " << root << std::endl;

	std::cout << "  Index Files: ";
	for (size_t i = 0; i < indexFiles.size(); i++)
		std::cout << indexFiles[i] << " ";
	std::cout << std::endl;

	std::cout << "  Allowed Methods: ";
	for (size_t i = 0; i < allowedMethods.size(); i++)
		std::cout << allowedMethods[i] << " ";
	std::cout << std::endl;

	std::cout << "  Auto Index: " << (autoIndex ? "Enabled" : "Disabled") << std::endl;
	
	std::cout << "  Max Body Size: " << maxBodySize << " bytes" << std::endl;

	if (!redirection.empty())
		std::cout << "  Redirection: " << redirection << std::endl;

	if (!cgiHandlers.empty())
	{
		std::cout << "  CGI Handlers: ";
		for (std::map<std::string, std::string>::const_iterator it = cgiHandlers.begin(); it != cgiHandlers.end(); ++it)
			std::cout << it->first << " -> " << it->second << " ";
		std::cout << std::endl;
	}

	if (!errorPages.empty())
	{
		std::cout << "  Error Pages: ";
		for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
			std::cout << it->first << " -> " << it->second << " ";
		std::cout << std::endl;
	}

	if (!headers.empty())
	{
		std::cout << "  Headers: ";
		for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
			std::cout << it->first << " -> " << it->second << " ";
		std::cout << std::endl;
	}

	if (!nestedLocations.empty())
	{
		std::cout << "  Nested Locations: " << std::endl;
		for (std::map<std::string, ServerConfigLocation>::const_iterator it = nestedLocations.begin(); it != nestedLocations.end(); ++it)
		{
			std::cout << "  LOCATION PRINT  " << std::endl;
			it->second.printLocationConfig();
		}
	}

	std::cout << "---------------------------" << std::endl;
}
