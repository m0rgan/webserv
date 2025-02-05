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
	{
		servers = other.servers;
	}
	return *this;
}

ConfigParser::~ConfigParser() {}

const std::vector<ServerConfig> &ConfigParser::getServers() const
{
	return servers;
}

std::vector<ServerConfig> ConfigParser::parseConfigFile(const std::string &filename)
{
	std::vector<ServerConfig> parsedServers;
	ServerConfig currentConfig;
	std::ifstream file(filename.c_str());

	if (!file)
	{
		std::cerr << "Error: Cannot open configuration file: " << filename << std::endl;
		return parsedServers;
	}

	std::string line;
	std::string currentLocation;

	while (std::getline(file, line))
	{
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
			{
				currentConfig.addPort(port);
			}
		}
		else if (key == "server_name")
		{
			std::string serverName;
			lineStream >> serverName;
			currentConfig.setServerName(serverName);
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
