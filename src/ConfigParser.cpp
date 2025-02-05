/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 13:00:30 by migumore          #+#    #+#             */
/*   Updated: 2025/02/05 13:17:55 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ConfigParser.hpp>

std::vector<ServerConfig> ConfigParser::parseConfigFile(const std::string &filename) {
    std::vector<ServerConfig> servers;
    ServerConfig currentConfig;
    std::ifstream file(filename.c_str());

    if (!file) {
        std::cerr << "Error: Cannot open configuration file: " << filename << std::endl;
        return servers;
    }

    std::string line;
    std::string currentLocation;

    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::string key;

        if (!(lineStream >> key)) continue; // Skip empty lines

        if (key == "server") {
			if (!currentConfig.ports.empty()) { // If ports exist, store the current server config
				servers.push_back(currentConfig);
				currentConfig = ServerConfig(); // Reset for a new server block
			}
		} else if (key == "listen") {
			int port;
			while (lineStream >> port) {
				currentConfig.ports.push_back(port);
			}
        } else if (key == "server_name") {
            lineStream >> currentConfig.serverName;
        } else if (key == "error_page") {
            int errorCode;
            std::string pagePath;
            lineStream >> errorCode >> pagePath;
            currentConfig.errorPages[errorCode] = pagePath;
        } else if (key == "index") {
            std::string file;
            while (lineStream >> file) {
                currentConfig.indexFiles.push_back(file);
            }
        } else if (key == "autoindex") {
            std::string value;
            lineStream >> value;
            currentConfig.autoIndex = (value == "on");
        } else if (key == "client_max_body_size") {
            lineStream >> currentConfig.maxBodySize;
        } else if (key == "client_timeout") {
            lineStream >> currentConfig.clientTimeout;
        } else if (key == "log_access") {
            lineStream >> currentConfig.logAccessFile;
        } else if (key == "log_error") {
            lineStream >> currentConfig.logErrorFile;
        } else if (key == "redirect") {
            std::string from, to;
            lineStream >> from >> to;
            currentConfig.redirects[from] = to;
        } else if (key == "upstream") {
            std::string backend;
            while (lineStream >> backend) {
                currentConfig.upstreamServers.push_back(backend);
            }
        } else if (key == "allow") {
            std::string ip;
            while (lineStream >> ip) {
                currentConfig.allowList.push_back(ip);
            }
        } else if (key == "deny") {
            std::string ip;
            while (lineStream >> ip) {
                currentConfig.denyList.push_back(ip);
            }
        } else if (key == "mime_type") {
            std::string mimeType, extension;
            lineStream >> mimeType;
            while (lineStream >> extension) {
                currentConfig.mimeTypes[extension] = mimeType;
            }
        }
    }

    servers.push_back(currentConfig); // Store the last parsed server block
    return servers;
}
