/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 13:00:30 by migumore          #+#    #+#             */
/*   Updated: 2025/04/02 18:46:53 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ConfigFile.hpp>

ConfigFile::ConfigFile() {}

ConfigFile::ConfigFile(const ConfigFile &src) //fix
{
	*this = src;
}

ConfigFile &ConfigFile::operator=(const ConfigFile &rhs) //fix
{
	if (this != &rhs)
		_parsedServers = rhs._parsedServers;
	return (*this);
}

ConfigFile::~ConfigFile() {}

const std::vector<ConfigFileServer> &ConfigFile::getServers() const
{
	return (_parsedServers);
}

void ConfigFile::parser(const std::string &filename)
{
	std::ifstream file(filename.c_str());
	if (!file)
		throw std::runtime_error("[ERROR] Cannot open configuration file: " + filename);
	std::string line;
	while (std::getline(file, line))
	{
		line = ignoreComments(line);
		std::istringstream lineStream(line);
		std::string key;
		if (!(lineStream >> key))
			continue; // Skip empty lines
		if (key == "server")
		{
			std::string remaining;
			if (lineStream >> remaining && remaining != "{")
				throw std::runtime_error("Invalid syntax, expected {");
			if (lineStream >> remaining)
				throw std::runtime_error("Invalid syntax after {");
			ConfigFileServer newServer = parseServerBlock(file);
			if (!isDuplicateServer(_parsedServers, newServer))
				_parsedServers.push_back(newServer);
		}
	}
}

ConfigFileServer ConfigFile::parseServerBlock(std::ifstream &file)
{
	ConfigFileServer configFileServer;
	std::string line;
	bool hasListenDirective = false;
	while (std::getline(file, line))
	{
		line = ignoreComments(line);
		std::istringstream lineStream(line);
		std::string key;
		if (!(lineStream >> key))
			continue;
		if (key == "}")
		{
			if (!hasListenDirective)
				parseListenDirective("80", configFileServer);
			break;
		}
		if (key == "location")
		{
			std::string locationPath;
			lineStream >> locationPath;
			std::string remaining;
			if (lineStream >> remaining && remaining != "{")
				throw std::runtime_error("Invalid syntax, expected { after location path");
			if (lineStream >> remaining)
				throw std::runtime_error("Unexpected characters after location block opening");
			ConfigFileServerLocation location = parseLocationBlock(file, locationPath);
			configFileServer.addLocation(location);
		}
		else
		{
			if (key == "listen")
				hasListenDirective = true;
			serverParseKeyValue(lineStream, key, configFileServer);
		}
	}
	if (configFileServer.hasDuplicateHostPortInBlock()) {
        throw std::runtime_error("[ERROR] Duplicate host:port for server: " + configFileServer.getServerName());
    }
	
	const std::map<std::string, ConfigFileServerLocation>& locations = configFileServer.getLocations();
	std::map<std::string, ConfigFileServerLocation>::const_iterator it = locations.find("/");
	if (it != locations.end())
	{
		const ConfigFileServerLocation& rootLocation = it->second;
		
		if (!rootLocation.getRoot().empty())
			configFileServer.setRoot(rootLocation.getRoot());
		
		if (!rootLocation.getIndexFiles().empty())
			configFileServer.setIndexFiles(rootLocation.getIndexFiles());
		
		if (rootLocation.getAutoIndex() != false)
			configFileServer.setAutoIndex(rootLocation.getAutoIndex());
		
		if (rootLocation.getMaxBodySize() != 1048576)
			configFileServer.setMaxBodySize(rootLocation.getMaxBodySize());
		
		if (rootLocation.hasReturnDirective())
		{
			configFileServer.addReturnDirective(
				rootLocation.getReturnStatusCode(),
				rootLocation.getReturnUrl()
			);
		}
	}
	return configFileServer;
}

ConfigFileServerLocation ConfigFile::parseLocationBlock(std::ifstream &file, const std::string &locationPath)
{
	ConfigFileServerLocation locationConfig(locationPath);
	std::string line;

	while (std::getline(file, line))
	{
		line = ignoreComments(line);
		std::istringstream lineStream(line);
		std::string key;
		if (!(lineStream >> key))
			continue;
		if (key == "}")
			break;
		locationParseKeyValue(lineStream, key, locationConfig);
	}
	return (locationConfig);
}

void ConfigFile::parseListenDirective(const std::string &listenValue, ConfigFileServer &configFileServer)
{
	std::string host = "0.0.0.0";
	int port = 80;

	size_t colonPos = listenValue.find(':');
	try
	{
		if (colonPos != std::string::npos)
		{
			if (listenValue[0] == '[')
			{
				size_t endBracketPos = listenValue.find(']');
				if (endBracketPos == std::string::npos || endBracketPos < colonPos)
					throw std::invalid_argument("Invalid IPv6 address format");

				host = listenValue.substr(1, endBracketPos - 1);
				std::stringstream portStream(listenValue.substr(endBracketPos + 2));
				if (!(portStream >> port))
					throw std::invalid_argument("Invalid port number");
				configFileServer.addHostPort(host, port);
			}
			else
			{
				host = listenValue.substr(0, colonPos);
				std::stringstream portStream(listenValue.substr(colonPos + 1));
				if (!(portStream >> port))
					throw std::invalid_argument("Invalid port number");
				configFileServer.addHostPort(host, port);
			}
		}
		else
		{
			std::stringstream portStream(listenValue);
			if (!(portStream >> port))
				throw std::invalid_argument("Invalid port number");
			configFileServer.addHostPort(host, port);
		}
	}
	catch (const std::invalid_argument &e)
	{
		std::cerr << "Error: Invalid port number in listen directive: " << listenValue << std::endl;
		return;
	}
	catch (const std::out_of_range &e)
	{
		std::cerr << "Error: Port number out of range in listen directive: " << listenValue << std::endl;
		return;
	}
}

void ConfigFile::serverParseKeyValue(std::istringstream &lineStream, const std::string &key, ConfigFileServer &configFileServer)
{
	if (key == "listen")
	{
		std::string listenValue;
		while (lineStream >> listenValue)
			parseListenDirective(listenValue, configFileServer);
	}
	else if (key == "server_name")
	{
		std::string serverName;
		lineStream >> serverName;
		configFileServer.setServerName(serverName);
	}
	else if (key == "root")
	{
		std::string root;
		lineStream >> root;
		configFileServer.setRoot(root);
	}
	else if (key == "error_page")
	{
		int errorCode;
		std::string pagePath;
		lineStream >> errorCode >> pagePath;
		configFileServer.addErrorPage(errorCode, pagePath);
	}
	else if (key == "index")
	{
		std::string file;
		while (lineStream >> file)
			configFileServer.addIndexFile(file);
	}
	else if (key == "autoindex")
	{
		std::string value;
		lineStream >> value;
		configFileServer.setAutoIndex(value == "on");
	}
	else if (key == "client_max_body_size")
	{
		std::string sizeStr;
		lineStream >> sizeStr;

		try
		{
			size_t maxSize = sizeConversion(sizeStr);
			configFileServer.setMaxBodySize(maxSize);
		}
		catch (const std::invalid_argument &e)
		{
			throw std::runtime_error("[ERROR] invalid client_max_body_size");
		}
	}
	else if (key == "return")
	{
		if (!configFileServer.hasReturnDirective())
		{
			int statusCode;
			std::string redirectUrl;
			lineStream >> statusCode >> redirectUrl;
			configFileServer.addReturnDirective(statusCode, redirectUrl);
		}
	}
	else
		throw std::invalid_argument("[ERROR] Unknown directive '" + key + "' in server configuration.");
}

void ConfigFile::locationParseKeyValue(std::istringstream &lineStream, const std::string &key, ConfigFileServerLocation &locationConfig)
{
	if (key == "root")
	{
		std::string root;
		lineStream >> root;
		locationConfig.setRoot(root);
	}
	else if (key == "alias")
    {
        std::string alias;
        lineStream >> alias;
        locationConfig.setAlias(alias);
    }
	else if (key == "index")
	{
		std::string file;
		while (lineStream >> file)
			locationConfig.addIndexFile(file);
	}
	else if (key == "autoindex")
	{
		std::string value;
		lineStream >> value;
		locationConfig.setAutoIndex(value == "on");
	}
	else if (key == "limit_except")
	{
		std::string method;
		while (lineStream >> method)
			locationConfig.addAllowedMethod(method);
	}
	else if (key == "error_page")
	{
		int errorCode;
		std::string pagePath;
		lineStream >> errorCode >> pagePath;
		locationConfig.addErrorPage(errorCode, pagePath);
	}
	else if (key == "client_max_body_size")
	{
		std::string sizeStr;
		lineStream >> sizeStr;

		try
		{
			size_t maxSize = sizeConversion(sizeStr);
			locationConfig.setMaxBodySize(maxSize);
		}
		catch (const std::invalid_argument &e)
		{
			throw std::runtime_error("[ERROR]] invalid client_max_body_size.");
		}
	}
	else if (key == "return")
	{
		if (!locationConfig.hasReturnDirective())
		{
			int statusCode;
			std::string redirectUrl;
			lineStream >> statusCode >> redirectUrl;
			locationConfig.addReturnDirective(statusCode, redirectUrl);
		}
	}
	else
		throw std::invalid_argument("[ERROR] Unknown directive '" + key + "' in server configuration.");
}

bool ConfigFile::isDuplicateServer(const std::vector<ConfigFileServer> &servers, const ConfigFileServer &newServer)
{
	for (size_t i = 0; i < servers.size(); ++i)
	{
		if (servers[i].getServerName() == newServer.getServerName())
		{
			const std::vector<std::pair<std::string, int> > &existingHostPorts = servers[i].getHostPort();
			const std::vector<std::pair<std::string, int> > &newHostPorts = newServer.getHostPort();
			for (size_t j = 0; j < existingHostPorts.size(); ++j)
			if (std::find(newHostPorts.begin(), newHostPorts.end(), existingHostPorts[j]) != newHostPorts.end())
					return (std::cerr << "[ERROR] Duplicate server_name + port found in configuration." << std::endl, true);
		}
	}
	return (false);
}

bool ConfigFile::hasDuplicateHostPort(const ConfigFileServer &configFileServer)
{
	const std::vector<std::pair<std::string, int> > &hostPortPairs = configFileServer.getHostPort();
	std::set<std::pair<std::string, int> > uniqueHostPorts;

	for (size_t i = 0; i < hostPortPairs.size(); ++i)
	{
		if (uniqueHostPorts.find(hostPortPairs[i]) != uniqueHostPorts.end())
			return (true);
		uniqueHostPorts.insert(hostPortPairs[i]);
	}
	return (false);
}

size_t ConfigFile::sizeConversion(const std::string &sizeStr)
{
	if (sizeStr.empty())
		throw std::invalid_argument("[ERROR] client_max_body_size cannot be empty.");

	std::stringstream ss(sizeStr);
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
			throw std::invalid_argument("[ERROR] Invalid unit in client_max_body_size: " + sizeStr);
	}
	return (value);
}

std::string &ConfigFile::ignoreComments(std::string &line)
{
	std::istringstream input(line);
	std::getline(input, line, '#');
	input.clear();
	input.str(line);
	std::getline(input, line, ';');
	return (line);
}

void ConfigFile::printConfig() const
{
	std::cout << "CONFIG FILE PRINT " << std::endl;
	for (size_t i = 0; i < _parsedServers.size(); ++i)
	{
		const ConfigFileServer &config = _parsedServers[i];
		std::cout << "Server " << i + 1 << ":" << std::endl;
		std::cout << "  Server Name: " << config.getServerName() << std::endl;
		std::cout << "  Host-Port Pairs: ";
		const std::vector<std::pair<std::string, int> > &hostPort = config.getHostPort();
		for (size_t j = 0; j < hostPort.size(); ++j)
		{
			std::cout << hostPort[j].first << ":" << hostPort[j].second;
			if (j < hostPort.size() - 1)
				std::cout << ", ";
		}
		std::cout << std::endl;
		std::cout << "  Root: " << config.getRoot() << std::endl;
		std::cout << "  Error Pages: ";
		const std::map<int, std::string> &errorPages = config.getErrorPages();
		for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
		{
			std::cout << it->first << " -> " << it->second;
			std::map<int, std::string>::const_iterator nxt = it;
			++nxt;
			if (nxt != errorPages.end())
				std::cout << ", ";
		}
		std::cout << std::endl;
		std::cout << "  Index Files: ";
		const std::vector<std::string> &indexFiles = config.getIndexFiles();
		for (size_t j = 0; j < indexFiles.size(); ++j)
		{
			std::cout << indexFiles[j];
			if (j < indexFiles.size() - 1)
				std::cout << ", ";
		}
		std::cout << std::endl;
		std::cout << "  Autoindex: " << (config.getAutoIndex() ? "on" : "off") << std::endl;
		std::cout << "  Max Body Size: " << config.getMaxBodySize() << std::endl;
		std::cout << "  Return Directives: ";
		if (config.hasReturnDirective())
		{
			const std::pair<int, std::string> &returnDirective = config.getReturnDirective();
			std::cout << returnDirective.first << " -> " << returnDirective.second;
		}
		std::cout << std::endl;
		const std::map<std::string, ConfigFileServerLocation> &locations = config.getLocations();
		for (std::map<std::string, ConfigFileServerLocation>::const_iterator it = locations.begin(); it != locations.end(); ++it)
			it->second.printLocationConfig();
		std::cout << "---------------------------" << std::endl;
		std::cout << std::endl;
	}
}