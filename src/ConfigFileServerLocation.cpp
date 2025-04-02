/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFileServerLocation.cpp                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/23 13:41:30 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/04/02 18:37:40 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigFileServerLocation.hpp"

ConfigFileServerLocation::ConfigFileServerLocation() : _autoIndex(false), _maxBodySize(1048576), _hasReturnDirective(false) {}
ConfigFileServerLocation::ConfigFileServerLocation(const std::string &uri) : _uri(uri), _autoIndex(false), _maxBodySize(1048576), _hasReturnDirective(false) {}
ConfigFileServerLocation::ConfigFileServerLocation(const ConfigFileServerLocation &src) {*this = src;} //fix
ConfigFileServerLocation &ConfigFileServerLocation::operator=(const ConfigFileServerLocation &rhs)
{
	if (this != &rhs)
	{
		this->_uri = rhs._uri;
		this->_alias = rhs._alias;
		this->_root = rhs._root;
		this->_indexFiles = rhs._indexFiles;
		this->_allowedMethods = rhs._allowedMethods;
		this->_autoIndex = rhs._autoIndex;
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

ConfigFileServerLocation::~ConfigFileServerLocation() {}

void ConfigFileServerLocation::setURI(const std::string &uri) { this->_uri = uri; }
const std::string &ConfigFileServerLocation::getURI() const { return this->_uri; }
void ConfigFileServerLocation::setRoot(const std::string &root) { this->_root = root; }
const std::string &ConfigFileServerLocation::getRoot() const { return this->_root; }
void ConfigFileServerLocation::addIndexFile(const std::string &file) { _indexFiles.push_back(file); }
const std::vector<std::string> &ConfigFileServerLocation::getIndexFiles() const { return _indexFiles; }
void ConfigFileServerLocation::setAutoIndex(bool enabled) { _autoIndex = enabled; }
bool ConfigFileServerLocation::getAutoIndex() const { return _autoIndex; }
void ConfigFileServerLocation::addAllowedMethod(const std::string &method) { _allowedMethods.push_back(method); }
const std::vector<std::string> &ConfigFileServerLocation::getAllowedMethods() const { return _allowedMethods; }
void ConfigFileServerLocation::addErrorPage(int errorCode, const std::string &pagePath) { _errorPages[errorCode] = pagePath; }
const std::map<int, std::string> &ConfigFileServerLocation::getErrorPages() const { return _errorPages; }
void ConfigFileServerLocation::addHeader(const std::string &key, const std::string &value) { _headers[key] = value; }
const std::map<std::string, std::string> &ConfigFileServerLocation::getHeaders() const { return _headers; }
void ConfigFileServerLocation::addReturnDirective(int statusCode, const std::string &url)
{
	_returnDirective = std::make_pair(statusCode, url);
	_hasReturnDirective = true;
}

bool ConfigFileServerLocation::hasReturnDirective() const {return _hasReturnDirective;}
int ConfigFileServerLocation::getReturnStatusCode() const {return _returnDirective.first;}
const std::string &ConfigFileServerLocation::getReturnUrl() const {return _returnDirective.second;}
const std::pair<int, std::string> &ConfigFileServerLocation::getReturnDirectives() const {return _returnDirective;}
void ConfigFileServerLocation::setMaxBodySize(size_t size) { _maxBodySize = size; }
size_t ConfigFileServerLocation::getMaxBodySize() const { return _maxBodySize; }
void ConfigFileServerLocation::addNestedLocation(const std::string &uri, const ConfigFileServerLocation &location){_nestedLocations[uri] = location;}
const std::map<std::string, ConfigFileServerLocation> &ConfigFileServerLocation::getNestedLocations() const {return _nestedLocations; }
void ConfigFileServerLocation::setAlias(const std::string &alias) { this->_alias = alias; }
const std::string &ConfigFileServerLocation::getAlias() const { return this->_alias; }
bool ConfigFileServerLocation::hasAlias() const { return !_alias.empty();}
bool ConfigFileServerLocation::matchesURI(const std::string &requestURI) const
{
	if (_uri.empty())
		return (false);
	// check for simple prefix match?
	return (requestURI.find(_uri) == 0);
}

void ConfigFileServerLocation::printLocationConfig() const
{
	std::cout << "---------------------------" << std::endl;
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
		for (std::map<std::string, ConfigFileServerLocation>::const_iterator it = _nestedLocations.begin(); it != _nestedLocations.end(); ++it)
		{
			std::cout << "  LOCATION PRINT  " << std::endl;
			it->second.printLocationConfig();
		}
	}
	std::cout << "    Return Directives: ";
	if (hasReturnDirective())
		std::cout << _returnDirective.first << " -> " << _returnDirective.second;
	std::cout << std::endl;
}
