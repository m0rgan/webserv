/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 15:16:44 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/26 22:50:36 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

ServerConfig::ServerConfig() : _maxBodySize(0), _autoIndex(false), _hasReturnDirective(false) {}

ServerConfig::ServerConfig(const ServerConfig &src){*this = src;} //FIX

ServerConfig &ServerConfig::operator=(const ServerConfig &rhs)
{
	if (this != &rhs)
	{
		this->_hostPort = rhs._hostPort;
		this->_ports = rhs._ports;
		this->_hosts = rhs._hosts;
		this->_name = rhs._name;
		this->_root = rhs._root;
		this->_errorPages = rhs._errorPages;
		this->_locations = rhs._locations;
		this->_indexFiles = rhs._indexFiles;
		this->_autoIndex = rhs._autoIndex;
		this->_maxBodySize = rhs._maxBodySize;
		this->_logAccessFile = rhs._logAccessFile;
		this->_logErrorFile = rhs._logErrorFile;
		this->_returnDirective = rhs._returnDirective;
		this->_hasReturnDirective = rhs._hasReturnDirective;
	}
	return (*this);
}

ServerConfig::~ServerConfig() {}

void ServerConfig::addPort(int port) { _ports.push_back(port); }
const std::vector<int> &ServerConfig::getPorts() const { return this->_ports; }
void ServerConfig::addHost(std::string host) { _hosts.push_back(host); }
const std::vector<std::string> &ServerConfig::getHosts() const { return this->_hosts; }
void ServerConfig::setServerName(const std::string &name) { _name = name; }
const std::string &ServerConfig::getName() const { return this->_name; }
void ServerConfig::setRoot(const std::string &root) { _root = root; }
const std::string &ServerConfig::getRoot() const { return this->_root; }
void ServerConfig::addErrorPage(int errorCode, const std::string &pagePath){_errorPages[errorCode] = pagePath;}
const std::map<int, std::string> &ServerConfig::getErrorPages() const { return this->_errorPages; }
void ServerConfig::addLocation(const ServerConfigLocation &location){_locations[location.getURI()] = location;}
const std::map<std::string, ServerConfigLocation> &ServerConfig::getLocations() const { return this->_locations; }
void ServerConfig::setAutoIndex(bool enabled) { _autoIndex = enabled; }
bool ServerConfig::getAutoIndex() const { return this->_autoIndex; }
void ServerConfig::addIndexFile(const std::string &file) { _indexFiles.push_back(file); }
const std::vector<std::string> &ServerConfig::getIndexFiles() const { return this->_indexFiles; }
void ServerConfig::setMaxBodySize(int size) { _maxBodySize = size; }
int ServerConfig::getMaxBodySize() const { return this->_maxBodySize; }
void ServerConfig::setLogAccessFile(const std::string &path) { _logAccessFile = path; }
const std::string &ServerConfig::getLogAccessFile() const { return this->_logAccessFile; }
void ServerConfig::setLogErrorFile(const std::string &path) { _logErrorFile = path; }
const std::string &ServerConfig::getLogErrorFile() const { return this->_logErrorFile; }
void ServerConfig::addReturnDirective(int statusCode, const std::string &url)
{
    _returnDirective = std::make_pair(statusCode, url);
    _hasReturnDirective = true;
}
bool ServerConfig::hasReturnDirective() const {return _hasReturnDirective;}
int ServerConfig::getReturnStatusCode() const {return _returnDirective.first;}
const std::string &ServerConfig::getReturnUrl() const {return _returnDirective.second;}
const std::pair<int, std::string> &ServerConfig::getReturnDirective() const {return _returnDirective;}
void ServerConfig::addHostPort(const std::string &host, int port) {_hostPort.push_back(std::make_pair(host, port));}
const std::vector<std::pair<std::string, int> > &ServerConfig::getHostPort() const { return _hostPort; }


void ServerConfig::printConfig() const
{
	std::cout << "SERVER CONFIG PRINT " << std::endl;
	std::cout << "=================================" << std::endl;
	std::cout << "Server Configuration: " << _name << std::endl;
	std::cout << "=================================" << std::endl;

	std::cout << "Listening on: ";
	for (size_t i = 0; i < _hostPort.size(); i++)
		std::cout << _hostPort[i].first << ":" << _hostPort[i].second << " ";
	std::cout << std::endl;

	std::cout << "Ports: ";
	for (size_t i = 0; i < _ports.size(); i++)
		std::cout << _ports[i] << " ";
	std::cout << std::endl;

	std::cout << "Hosts: ";
	for (size_t i = 0; i < _hosts.size(); i++)
		std::cout << _hosts[i] << " ";
	std::cout << std::endl;

	std::cout << "Root Directory: " << _root << std::endl;
	std::cout << "Auto Index: " << (_autoIndex ? "Enabled" : "Disabled") << std::endl;
	std::cout << "Max Body Size: " << _maxBodySize << " bytes" << std::endl;

	std::cout << "Index Files: ";
	for (size_t i = 0; i < _indexFiles.size(); i++)
		std::cout << _indexFiles[i] << " ";
	std::cout << std::endl;

	std::cout << "Error Pages: ";
	for (std::map<int, std::string>::const_iterator it = _errorPages.begin(); it != _errorPages.end(); ++it)
		std::cout << it->first << " -> " << it->second << " ";
	std::cout << std::endl;

	std::cout << "Log Files: " << std::endl;
	std::cout << "  Access Log: " << _logAccessFile << std::endl;
	std::cout << "  Error Log: " << _logErrorFile << std::endl;

	//need to print returns 
	// std::cout << "Redirections: ";
	// for (std::map<std::string, std::string>::const_iterator it = _redirects.begin(); it != _redirects.end(); ++it)
	// 	std::cout << it->first << " -> " << it->second << " ";
	// std::cout << std::endl;

	std::cout << "=================================" << std::endl;
	std::cout << "Location Blocks:" << std::endl;
	std::cout << "=================================" << std::endl;
	
	for (std::map<std::string, ServerConfigLocation>::const_iterator it = _locations.begin(); it != _locations.end(); ++it)
		it->second.printLocationConfig();

	std::cout << "=================================" << std::endl;
}
