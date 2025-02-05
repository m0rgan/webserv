/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 13:01:34 by migumore          #+#    #+#             */
/*   Updated: 2025/02/05 13:17:02 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ServerConfig.hpp>
#include <iostream>

ServerConfig::ServerConfig() : autoIndex(false), maxBodySize(1048576), clientTimeout(30) {
    ports.clear(); // Ensure ports vector is empty initially
}

ServerConfig::~ServerConfig() {}

void ServerConfig::printConfig() const {
    std::cout << "Server Name: " << serverName << std::endl;
    std::cout << "Ports: ";
	for (size_t i = 0; i < ports.size(); i++) {
		std::cout << ports[i] << " ";
	}
	std::cout << std::endl;


    std::cout << "Error Pages:" << std::endl;
    for (std::map<int, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it) {
        std::cout << "  " << it->first << " -> " << it->second << std::endl;
    }

    std::cout << "Index Files: ";
    for (size_t i = 0; i < indexFiles.size(); i++) {
        std::cout << indexFiles[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "Auto-Index: " << (autoIndex ? "on" : "off") << std::endl;
    std::cout << "Max Body Size: " << maxBodySize << " bytes" << std::endl;
    std::cout << "Client Timeout: " << clientTimeout << " seconds" << std::endl;
    std::cout << "Logging: Access: " << logAccessFile << ", Error: " << logErrorFile << std::endl;

    std::cout << "Redirects:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = redirects.begin(); it != redirects.end(); ++it) {
        std::cout << "  " << it->first << " -> " << it->second << std::endl;
    }

    std::cout << "Allowed MIME Types:" << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = mimeTypes.begin(); it != mimeTypes.end(); ++it) {
        std::cout << "  " << it->first << " -> " << it->second << std::endl;
    }
}
