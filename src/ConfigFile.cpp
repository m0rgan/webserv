/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 13:00:30 by migumore          #+#    #+#             */
/*   Updated: 2025/02/25 14:53:32 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ConfigFile.hpp>

ConfigFile::ConfigFile(const std::string &configFile)
{
	_serverBlockConfig = parser(configFile);
}

ConfigFile::ConfigFile(const ConfigFile &src) //fix
{
	*this = src;
}

ConfigFile &ConfigFile::operator=(const ConfigFile &rhs) //fix
{
	if (this != &rhs)
		_serverBlockConfig = rhs._serverBlockConfig;
	return (*this);
}

ConfigFile::~ConfigFile() {}

const std::vector<ServerConfig> &ConfigFile::getServers() const
{
	return (_serverBlockConfig);
}

bool ConfigFile::isDuplicateServer(const std::vector<ServerConfig> &servers, const ServerConfig &newServer)
{
	size_t	i;
	size_t	j;

	for (i = 0; i < servers.size(); ++i)
	{
		if (servers[i].getName() == newServer.getName())
		{
			const std::vector<int> &ports = servers[i].getPorts();
			for (j = 0; j < ports.size(); ++j)
				if (std::find(newServer.getPorts().begin(), newServer.getPorts().end(), ports[j]) != newServer.getPorts().end())
					return (std::cerr << "Error: Duplicate server_name + port found in configuration." << std::endl, true);
		}
	}
	return (false);
}

std::vector<ServerConfig> ConfigFile::parser(const std::string &filename)
{
	std::vector<ServerConfig> parsedServers;
	std::ifstream file(filename.c_str());
	if (!file)
	{
		std::cerr << "Error: Cannot open configuration file: " << filename << std::endl;
		return (parsedServers);
	}
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
			ServerConfig newServer = parseServerBlock(file);
			if (!isDuplicateServer(parsedServers, newServer))
				parsedServers.push_back(newServer);
		}
	}
	return (parsedServers);
}

ServerConfig ConfigFile::parseServerBlock(std::ifstream &file)
{
	ServerConfig serverConfig;
	std::string line;

	while (std::getline(file, line))
	{
		line = ignoreComments(line);
		std::istringstream lineStream(line);
		std::string key;
		if (!(lineStream >> key))
			continue; // Skip empty lines
		if (key == "}")
			break; // End of server block
		if (key == "location")
		{
			std::string locationPath;
			lineStream >> locationPath;
			ServerConfigLocation location = parseLocationBlock(file, locationPath);
			serverConfig.addLocation(location);
		}
		else
			serverParseKeyValue(lineStream, key, serverConfig);
	}
	return (serverConfig);
}

ServerConfigLocation ConfigFile::parseLocationBlock(std::ifstream &file, const std::string &locationPath)
{
	ServerConfigLocation locationConfig(locationPath);
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

void ConfigFile::parseListenDirective(const std::string &listenValue, ServerConfig &serverConfig)
{
	//invalid port number is simply returning but allows exectuion, does nginx launch anyway if error happens on port?
	std::string host = "0.0.0.0";
	int port = 80;

	size_t colonPos = listenValue.find(':');
	try
	{
		if (colonPos != std::string::npos)
		{
			if (listenValue[0] == '[') // IPv6 address
			{
				size_t endBracketPos = listenValue.find(']');
				if (endBracketPos == std::string::npos || endBracketPos < colonPos)
					throw std::invalid_argument("Invalid IPv6 address format");

				host = listenValue.substr(1, endBracketPos - 1); // Extract IPv6 address without brackets
				std::stringstream portStream(listenValue.substr(endBracketPos + 2));
				if (!(portStream >> port))
					throw std::invalid_argument("Invalid port number");
				// serverConfig.addHost("[" + host + "]");
				serverConfig.addHostPort("[" + host + "]", port);
			}
			else // IPv4 address
			{
				host = listenValue.substr(0, colonPos);
				std::stringstream portStream(listenValue.substr(colonPos + 1));
				if (!(portStream >> port))
					throw std::invalid_argument("Invalid port number");
				serverConfig.addHostPort(host, port);
			}
		}
		else
		{
			std::stringstream portStream(listenValue);
			if (!(portStream >> port))
				throw std::invalid_argument("Invalid port number");
			serverConfig.addHostPort(host, port);
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

	serverConfig.addPort(port);
	serverConfig.addHost(host);
}

void ConfigFile::serverParseKeyValue(std::istringstream &lineStream, const std::string &key, ServerConfig &serverConfig)
{
	if (key == "listen") //if doesnt exist make default or error?
	{
		std::string listenValue;
		while (lineStream >> listenValue)
			parseListenDirective(listenValue, serverConfig);
	}
	else if (key == "server_name")
	{
		std::string serverName;
		lineStream >> serverName;
		serverConfig.setServerName(serverName);
	}
	else if (key == "root")
	{
		std::string root;
		lineStream >> root;
		serverConfig.setRoot(root);
	}
	else if (key == "error_page")
	{
		int errorCode;
		std::string pagePath;
		lineStream >> errorCode >> pagePath;
		serverConfig.addErrorPage(errorCode, pagePath);
	}
	else if (key == "index")
	{
		std::string file;
		while (lineStream >> file)
			serverConfig.addIndexFile(file);
	}
	else if (key == "autoindex")
	{
		std::string value;
		lineStream >> value;
		serverConfig.setAutoIndex(value == "on"); //may not need this?
	}
	else if (key == "client_max_body_size")
	{
		int size;
		lineStream >> size;
		serverConfig.setMaxBodySize(size);
	}
	else if (key == "log_access")
	{
		std::string path;
		lineStream >> path;
		serverConfig.setLogAccessFile(path);
	}
	else if (key == "log_error")
	{
		std::string path;
		lineStream >> path;
		serverConfig.setLogErrorFile(path);
	}
	else if (key == "redirect")
	{
		std::string from, to;
		lineStream >> from >> to;
		serverConfig.addRedirect(from, to);
	}
}

void ConfigFile::locationParseKeyValue(std::istringstream &lineStream, const std::string &key, ServerConfigLocation &locationConfig)
{
	if (key == "root")
	{
		std::string root;
		lineStream >> root;
		locationConfig.setRoot(root);
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
	else if (key == "proxy_pass")
	{
		std::string proxyPass;
		lineStream >> proxyPass;
		locationConfig.setProxyPass(proxyPass);
	}
	else if (key == "proxy_pass")
	{
		std::string proxyPass;
		lineStream >> proxyPass;
		locationConfig.setProxyPass(proxyPass);
	}
}

// 5. Exposing Nginx to the Internet

// If you want Nginx to be accessible from outside your local network:

// Get your public IP (curl ifconfig.me or check your router's settings).
// Port Forwarding:
// Log into your router's admin panel.
// Forward port 80 (HTTP) and 443 (HTTPS) to your PC’s local IP.
// Check your public IP: Try accessing http://your-public-ip.
// ⚠️ Security Warning: If exposing to the internet, secure it with a firewall, SSL (Let’s Encrypt), and authentication.

// Change listen directive to allow all IPs:
// server {
//     listen 80;
//     server_name _;

//     root /var/www/html;
//     index index.html;
// }
// listen 80; → This tells Nginx to accept connections on port 80 from any IP.
// server_name _; → This ensures Nginx responds to all requests.
// Alternatively, specify a specific network interface IP:

// listen 192.168.1.100:80;
// Replace 192.168.1.100 with your machine’s LAN IP.

// nginx handles $variables so i must add logic for that

//location blocks can be several inside a server block, must handle
// location / {
// 	try_files $uri $uri/ =404;
// }
//must add try_files??s
// must add these:
// location /api/ {
//     proxy_pass http://backend_server;
//     proxy_set_header Host $host;
//     proxy_set_header X-Real-IP $remote_addr;
// }

//location ~ \.php$ {
//     fastcgi_pass unix:/run/php/php7.4-fpm.sock;
//     fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
//     include fastcgi_params;
// }

// location /ws/ {
//     proxy_pass http://backend;
//     proxy_http_version 1.1;
//     proxy_set_header Upgrade $http_upgrade;
//     proxy_set_header Connection "Upgrade";
// }

// #location ~* \.php$ this * is making nginx be not case sensitive so it can serve .PHP files

// # this root is problematic because of the empy spaces .....  /folder   123/folder2/file.html

std::string &ConfigFile::ignoreComments(std::string &line)
{
	std::istringstream input(line);
	std::getline(input, line, ';');
	input.clear();
	input.str(line);
	std::getline(input, line, '#');
	return (line);
}

void ConfigFile::printConfig() const
{
	std::cout << "CONFIG FILE PRINT " << std::endl;
	for (size_t i = 0; i < _serverBlockConfig.size(); ++i)
	{
		const ServerConfig &config = _serverBlockConfig[i];
		std::cout << "Server " << i + 1 << ":" << std::endl;
		std::cout << "  Server Name: " << config.getName() << std::endl;
		std::cout << "  Ports: ";
		const std::vector<int> &ports = config.getPorts();
		for (size_t j = 0; j < ports.size(); ++j)
		{
			std::cout << ports[j];
			if (j < ports.size() - 1)
				std::cout << ", ";
		}
		std::cout << std::endl;
		std::cout << "  Hosts: ";
		const std::vector<std::string> &hosts = config.getHosts();
		for (size_t j = 0; j < hosts.size(); ++j)
		{
			std::cout << hosts[j];
			if (j < hosts.size() - 1)
				std::cout << ", ";
		}
		std::cout << std::endl;
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
		std::cout << "  Log Access File: " << config.getLogAccessFile() << std::endl;
		std::cout << "  Log Error File: " << config.getLogErrorFile() << std::endl;
		std::cout << "  Redirects: ";
		const std::map<std::string, std::string> &redirects = config.getRedirects();
		for (std::map<std::string, std::string>::const_iterator it = redirects.begin(); it != redirects.end(); ++it)
		{
			std::cout << it->first << " -> " << it->second;
			std::map<std::string, std::string>::const_iterator nxt = it;
			++nxt;
			if (nxt != redirects.end())
				std::cout << ", ";
		}
		std::cout << std::endl;
		const std::map<std::string, ServerConfigLocation> &locations = config.getLocations();
		for (std::map<std::string, ServerConfigLocation>::const_iterator it = locations.begin(); it != locations.end(); ++it)
		{
			std::cout << "  Location: " << it->first << std::endl;
			it->second.printLocationConfig();
		}

		std::cout << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
}
}