/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 13:00:30 by migumore          #+#    #+#             */
/*   Updated: 2025/02/05 18:36:12 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ConfigParser.hpp>

ConfigParser::ConfigParser(const std::string &filename)
{
	servers = parseConfigFile(filename);
}

ConfigParser::ConfigParser(const ConfigParser &other)
{
	*this = other;
}

ConfigParser &ConfigParser::operator=(const ConfigParser &other)
{
	if (this != &other)
		servers = other.servers;
	return *this;
}

ConfigParser::~ConfigParser() {}

const std::vector<ServerConfig> &ConfigParser::getServers() const
{
	return servers;
}

// only add unique host+port pairs???

std::vector<ServerConfig> ConfigParser::parseConfigFile(const std::string &filename)
{
	std::vector<ServerConfig> parsedServers;
	ServerConfig currentConfig;
	std::ifstream file(filename.c_str());
	//should use access to check for file permissions or is not neccesary with ifstream etc?
	if (!file)
	{
		std::cerr << "Error: Cannot open configuration file: " << filename << std::endl;
		return parsedServers;
	}

	std::string line;
	std::string currentLocation;

	while (std::getline(file, line))
	{
		line = ignoreComments(line);
		std::istringstream lineStream(line);
		std::string key;

		if (!(lineStream >> key))
			continue; // Skip empty lines

		if (key == "server")
		{
			if (!currentConfig.getPorts().empty())
			{
				parsedServers.push_back(currentConfig);
				currentConfig = ServerConfig(); // Reset for new server block
			}
		}
		else if (key == "listen")
		{
			int port;
			while (lineStream >> port)
				currentConfig.addPort(port);
		}
		else if (key == "server_name")
		{
			std::string serverName;
			lineStream >> serverName;
			currentConfig.setServerName(serverName);
		}
		else if (key == "root")
		{
            std::string root;
            lineStream >> root;
            currentConfig.setRoot(root);
		}
		else if (key == "error_page")
		{
			int errorCode;
			std::string pagePath;
			lineStream >> errorCode >> pagePath;
			currentConfig.addErrorPage(errorCode, pagePath);
		}
		else if (key == "index")
		{
			std::string file;
			while (lineStream >> file)
			{
				currentConfig.addIndexFile(file);
			}
		}
		else if (key == "autoindex")
		{
			std::string value;
			lineStream >> value;
			currentConfig.setAutoIndex(value == "on");
		}
		else if (key == "client_max_body_size")
		{
			int size;
			lineStream >> size;
			currentConfig.setMaxBodySize(size);
		}
		else if (key == "client_timeout")
		{
			int timeout;
			lineStream >> timeout;
			currentConfig.setClientTimeout(timeout);
		}
		else if (key == "log_access")
		{
			std::string path;
			lineStream >> path;
			currentConfig.setLogAccessFile(path);
		}
		else if (key == "log_error")
		{
			std::string path;
			lineStream >> path;
			currentConfig.setLogErrorFile(path);
		}
		else if (key == "redirect")
		{
			std::string from, to;
			lineStream >> from >> to;
			currentConfig.addRedirect(from, to);
		}
		else if (key == "upstream")
		{
			std::string backend;
			while (lineStream >> backend)
			{
				currentConfig.addUpstreamServer(backend);
			}
		}
		else if (key == "allow")
		{
			std::string ip;
			while (lineStream >> ip)
			{
				currentConfig.addAllowIP(ip);
			}
		}
		else if (key == "deny")
		{
			std::string ip;
			while (lineStream >> ip)
			{
				currentConfig.addDenyIP(ip);
			}
		}
		else if (key == "mime_type")
		{
			std::string mimeType, extension;
			lineStream >> mimeType;
			while (lineStream >> extension)
			{
				currentConfig.addMimeType(extension, mimeType);
			}
		}
	}

	parsedServers.push_back(currentConfig); // Store last parsed server block
	return parsedServers;
}

std::string &ConfigParser::ignoreComments(std::string &line)
{
	std::istringstream input(line);
	std::getline(input, line, ';');
	input.clear();
	input.str(line);
	std::getline(input, line, '#');
	return (line);
}

void ConfigParser::printConfig() const
{
    for (size_t i = 0; i < servers.size(); ++i)
    {
        const ServerConfig &config = servers[i];
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
        std::cout << "  Root: " << config.getRoot() << std::endl;
        std::cout << "  Error Pages: ";
        const std::map<int, std::string> &errorPages = config.getErrorPages();
        for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it)
        {
            std::cout << it->first << " -> " << it->second;
            if (std::next(it) != errorPages.end())
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
        // std::cout << "  Autoindex: " << (config.getAutoIndex() ? "on" : "off") << std::endl;
        std::cout << "  Max Body Size: " << config.getMaxBodySize() << std::endl;
        std::cout << "  Client Timeout: " << config.getClientTimeout() << std::endl;
        std::cout << "  Log Access File: " << config.getLogAccessFile() << std::endl;
        std::cout << "  Log Error File: " << config.getLogErrorFile() << std::endl;
        std::cout << "  Redirects: ";
        const std::map<std::string, std::string> &redirects = config.getRedirects();
        for (std::map<std::string, std::string>::const_iterator it = redirects.begin(); it != redirects.end(); ++it)
        {
            std::cout << it->first << " -> " << it->second;
            if (std::next(it) != redirects.end())
                std::cout << ", ";
        }
        std::cout << std::endl;
        std::cout << "  Upstream Servers: ";
        const std::vector<std::string> &upstreamServers = config.getUpstreamServers();
        for (size_t j = 0; j < upstreamServers.size(); ++j)
        {
            std::cout << upstreamServers[j];
            if (j < upstreamServers.size() - 1)
                std::cout << ", ";
        }
        std::cout << std::endl;
        // std::cout << "  Allowed IPs: ";
        // // const std::vector<std::string> &allowIPs = config.getAllowIPs();
        // for (size_t j = 0; j < allowIPs.size(); ++j)
        // {
        //     std::cout << allowIPs[j];
        //     if (j < allowIPs.size() - 1)
        //         std::cout << ", ";
        // }
        // std::cout << std::endl;
        // std::cout << "  Denied IPs: ";
        // // const std::vector<std::string> &denyIPs = config.getDenyIPs();
        // for (size_t j = 0; j < denyIPs.size(); ++j)
        // {
        //     std::cout << denyIPs[j];
        //     if (j < denyIPs.size() - 1)
        //         std::cout << ", ";
        // }
        // std::cout << std::endl;
        std::cout << "  MIME Types: ";
        const std::map<std::string, std::string> &mimeTypes = config.getMimeTypes();
        for (std::map<std::string, std::string>::const_iterator it = mimeTypes.begin(); it != mimeTypes.end(); ++it)
        {
            std::cout << it->first << " -> " << it->second;
            if (std::next(it) != mimeTypes.end())
			std::cout << ", ";
	}
	std::cout << std::endl;
	std::cout << std::endl;
}
}