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

ServerConfigLocation::ServerConfigLocation() : _autoIndex(false), _maxBodySize(1048576), _hasReturnDirective(false) {}
ServerConfigLocation::ServerConfigLocation(const std::string &uri) : _uri(uri), _autoIndex(false), _maxBodySize(1048576), _hasReturnDirective(false) {}
ServerConfigLocation::ServerConfigLocation(const ServerConfigLocation &src) {*this = src;} //fix
ServerConfigLocation &ServerConfigLocation::operator=(const ServerConfigLocation &rhs)
{
	if (this != &rhs)
	{
		this->_uri = rhs._uri;
		this->_alias = rhs._alias;
		this->_root = rhs._root;
		this->_indexFiles = rhs._indexFiles;
		this->_allowedMethods = rhs._allowedMethods;
		this->_autoIndex = rhs._autoIndex;
		this->_cgiHandlers = rhs._cgiHandlers;
		this->_errorPages = rhs._errorPages;
		this->_headers = rhs._headers;
		this->_returnDirective = rhs._returnDirective;
		this->_hasReturnDirective = rhs._hasReturnDirective;
		this->_maxBodySize = rhs._maxBodySize;
		this->_nestedLocations = rhs._nestedLocations;
		//some are missing
	}
	return (*this);
}

ServerConfigLocation::~ServerConfigLocation() {}

void ServerConfigLocation::setURI(const std::string &uri) { this->_uri = uri; }
const std::string &ServerConfigLocation::getURI() const { return this->_uri; }
void ServerConfigLocation::setRoot(const std::string &root) { this->_root = root; }
const std::string &ServerConfigLocation::getRoot() const { return this->_root; }
void ServerConfigLocation::addIndexFile(const std::string &file) { _indexFiles.push_back(file); }
const std::vector<std::string> &ServerConfigLocation::getIndexFiles() const { return _indexFiles; }
void ServerConfigLocation::setAutoIndex(bool enabled) { _autoIndex = enabled; }
bool ServerConfigLocation::getAutoIndex() const { return _autoIndex; }
void ServerConfigLocation::setProxyPass(const std::string &proxyPass) { this->_proxyPass = proxyPass; }
const std::string &ServerConfigLocation::getProxyPass() const { return _proxyPass; }
void ServerConfigLocation::addAllowedMethod(const std::string &method) { _allowedMethods.push_back(method); }
const std::vector<std::string> &ServerConfigLocation::getAllowedMethods() const { return _allowedMethods; }
void ServerConfigLocation::addCgiHandler(const std::string &extension, const std::string &handler) { _cgiHandlers[extension] = handler; }
const std::map<std::string, std::string> &ServerConfigLocation::getCgiHandlers() const { return _cgiHandlers; }
void ServerConfigLocation::addErrorPage(int errorCode, const std::string &pagePath) { _errorPages[errorCode] = pagePath; }
const std::map<int, std::string> &ServerConfigLocation::getErrorPages() const { return _errorPages; }
void ServerConfigLocation::addHeader(const std::string &key, const std::string &value) { _headers[key] = value; }
const std::map<std::string, std::string> &ServerConfigLocation::getHeaders() const { return _headers; }
void ServerConfigLocation::addReturnDirective(int statusCode, const std::string &url)
{
	_returnDirective = std::make_pair(statusCode, url);
	_hasReturnDirective = true;
}

bool ServerConfigLocation::hasReturnDirective() const {return _hasReturnDirective;}
int ServerConfigLocation::getReturnStatusCode() const {return _returnDirective.first;}
const std::string &ServerConfigLocation::getReturnUrl() const {return _returnDirective.second;}
const std::pair<int, std::string> &ServerConfigLocation::getReturnDirectives() const {return _returnDirective;}
void ServerConfigLocation::setMaxBodySize(size_t size) { _maxBodySize = size; }
size_t ServerConfigLocation::getMaxBodySize() const { return _maxBodySize; }
void ServerConfigLocation::addNestedLocation(const std::string &uri, const ServerConfigLocation &location){_nestedLocations[uri] = location;}
const std::map<std::string, ServerConfigLocation> &ServerConfigLocation::getNestedLocations() const {return _nestedLocations; }
void ServerConfigLocation::setAlias(const std::string &alias) { this->_alias = alias; }
const std::string &ServerConfigLocation::getAlias() const { return this->_alias; }

bool ServerConfigLocation::matchesURI(const std::string &requestURI) const
{
	if (_uri.empty())
		return (false);
	// check for simple prefix match?
	return (requestURI.find(_uri) == 0);
}

void ServerConfigLocation::printLocationConfig() const
{
	std::cout << "  Location: " << _uri << std::endl;
	std::cout << "  Root: " << _root << std::endl;

	std::cout << "  Index Files: ";
	for (size_t i = 0; i < _indexFiles.size(); i++)
		std::cout << _indexFiles[i] << " ";
	std::cout << std::endl;

	std::cout << "  Allowed Methods: ";
	for (size_t i = 0; i < _allowedMethods.size(); i++)
		std::cout << _allowedMethods[i] << " ";
	std::cout << std::endl;

	std::cout << "  Auto Index: " << (_autoIndex ? "Enabled" : "Disabled") << std::endl;
	
	std::cout << "  Max Body Size: " << _maxBodySize << " bytes" << std::endl;

	if (!_cgiHandlers.empty())
	{
		std::cout << "  CGI Handlers: ";
		for (std::map<std::string, std::string>::const_iterator it = _cgiHandlers.begin(); it != _cgiHandlers.end(); ++it)
			std::cout << it->first << " -> " << it->second << " ";
		std::cout << std::endl;
	}

	if (!_errorPages.empty())
	{
		std::cout << "  Error Pages: ";
		for (std::map<int, std::string>::const_iterator it = _errorPages.begin(); it != _errorPages.end(); ++it)
			std::cout << it->first << " -> " << it->second << " ";
		std::cout << std::endl;
	}

	if (!_headers.empty())
	{
		std::cout << "  Headers: ";
		for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
			std::cout << it->first << " -> " << it->second << " ";
		std::cout << std::endl;
	}

	if (!_nestedLocations.empty())
	{
		std::cout << "  Nested Locations: " << std::endl;
		for (std::map<std::string, ServerConfigLocation>::const_iterator it = _nestedLocations.begin(); it != _nestedLocations.end(); ++it)
		{
			std::cout << "  LOCATION PRINT  " << std::endl;
			it->second.printLocationConfig();
		}
	}
	std::cout << "    Proxy Pass: " << getProxyPass() << std::endl;
	std::cout << "    Return Directives: ";
	if (hasReturnDirective())
		std::cout << _returnDirective.first << " -> " << _returnDirective.second;
	std::cout << std::endl;
	std::cout << "---------------------------" << std::endl;
}
