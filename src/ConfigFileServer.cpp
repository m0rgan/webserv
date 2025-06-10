/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFileServer.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/23 13:39:45 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/04/02 18:14:47 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigFileServer.hpp"

ConfigFileServer::ConfigFileServer() : _maxBodySize(1048576), _autoIndex(false), _hasReturnDirective(false) {}

ConfigFileServer::ConfigFileServer(const ConfigFileServer &src){*this = src;} //FIX

ConfigFileServer &ConfigFileServer::operator=(const ConfigFileServer &rhs)
{
	if (this != &rhs)
	{
		this->_hostPort = rhs._hostPort;
		this->_serverName = rhs._serverName;
		this->_root = rhs._root;
		this->_errorPages = rhs._errorPages;
		this->_locations = rhs._locations;
		this->_indexFiles = rhs._indexFiles;
		this->_autoIndex = rhs._autoIndex;
		this->_maxBodySize = rhs._maxBodySize;
		this->_returnDirective = rhs._returnDirective;
		this->_hasReturnDirective = rhs._hasReturnDirective;
	}
	return (*this);
}

ConfigFileServer::~ConfigFileServer() {}

void ConfigFileServer::setServerName(const std::string &name) { _serverName = name; }
const std::string &ConfigFileServer::getServerName() const { return this->_serverName; }
void ConfigFileServer::setRoot(const std::string &root) { _root = root; }
const std::string &ConfigFileServer::getRoot() const { return this->_root; }
void ConfigFileServer::addErrorPage(int errorCode, const std::string &pagePath){_errorPages[errorCode] = pagePath;}
const std::map<int, std::string> &ConfigFileServer::getErrorPages() const { return this->_errorPages; }
void ConfigFileServer::addLocation(const ConfigFileServerLocation &location){_locations[location.getURI()] = location;}
const std::map<std::string, ConfigFileServerLocation> &ConfigFileServer::getLocations() const { return this->_locations; }
void ConfigFileServer::setAutoIndex(bool enabled) { _autoIndex = enabled; }
bool ConfigFileServer::getAutoIndex() const { return this->_autoIndex; }
void ConfigFileServer::addIndexFile(const std::string &file) { _indexFiles.push_back(file); }
void ConfigFileServer::setIndexFiles(const std::vector<std::string> &files) { _indexFiles.clear(); _indexFiles = files; }
const std::vector<std::string> &ConfigFileServer::getIndexFiles() const { return this->_indexFiles; }
void ConfigFileServer::setMaxBodySize(size_t size) { _maxBodySize = size; }
size_t ConfigFileServer::getMaxBodySize() const { return this->_maxBodySize; }
void ConfigFileServer::addReturnDirective(int statusCode, const std::string &url) { _returnDirective = std::make_pair(statusCode, url); _hasReturnDirective = true; }
bool ConfigFileServer::hasReturnDirective() const {return _hasReturnDirective;}
int ConfigFileServer::getReturnStatusCode() const {return _returnDirective.first;}
const std::string &ConfigFileServer::getReturnUrl() const {return _returnDirective.second;}
const std::pair<int, std::string> &ConfigFileServer::getReturnDirective() const {return _returnDirective;}
void ConfigFileServer::addHostPort(const std::string &host, int port) {_hostPort.push_back(std::make_pair(host, port));}
const std::vector<std::pair<std::string, int> > &ConfigFileServer::getHostPort() const { return _hostPort; }

bool ConfigFileServer::hasDuplicateHostPortInBlock() const {
    std::set<std::pair<std::string, int> > uniqueHostPorts;

    for (std::vector<std::pair<std::string, int> >::const_iterator it = _hostPort.begin();
         it != _hostPort.end(); ++it)
    {
        if (uniqueHostPorts.find(*it) != uniqueHostPorts.end()) {
            return true;
        }
        uniqueHostPorts.insert(*it);
    }
    return false;
}