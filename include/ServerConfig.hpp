/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 12:59:38 by migumore          #+#    #+#             */
/*   Updated: 2025/02/05 13:14:17 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>
#include <map>

class ServerConfig
{
public:
    std::vector<int> ports;
    std::string serverName;
    std::map<int, std::string> errorPages; // 404 -> "/errors/404.html"
    std::map<std::string, std::string> locations; // URI -> root directory
    std::map<std::string, std::vector<std::string> > methods; // URI -> allowed methods
    std::string cgiExtension;

    // New features
    std::vector<std::string> indexFiles;
    bool autoIndex;
    int maxBodySize;
    int clientTimeout;
    std::string logAccessFile;
    std::string logErrorFile;
    std::map<std::string, std::string> redirects; // URI -> target
    std::vector<std::string> upstreamServers;
    std::vector<std::string> allowList;
    std::vector<std::string> denyList;
    std::map<std::string, std::string> mimeTypes; // Extension -> MIME Type

    ServerConfig();
    ~ServerConfig();
    void printConfig() const;
};

#endif // SERVERCONFIG_HPP
