/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 12:59:38 by migumore          #+#    #+#             */
/*   Updated: 2025/02/05 18:29:10 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <vector>
#include <map>

class ServerConfig
{
private:
	std::vector<int> ports;
	std::string _name;
	std::map<int, std::string> errorPages;
	std::map<std::string, std::string> locations;
	std::map<std::string, std::vector<std::string> > methods;
	std::string cgiExtension;
	std::vector<std::string> indexFiles;
	bool autoIndex;
	int maxBodySize;
	int clientTimeout;
	std::string logAccessFile;
	std::string logErrorFile;
	std::map<std::string, std::string> redirects;
	std::vector<std::string> upstreamServers;
	std::vector<std::string> allowList;
	std::vector<std::string> denyList;
	std::map<std::string, std::string> mimeTypes;

public:
	ServerConfig();										// Default Constructor
	ServerConfig(const ServerConfig &other);			// Copy Constructor
	ServerConfig &operator=(const ServerConfig &other); // Copy Assignment Operator
	~ServerConfig();									// Destructor

	const std::vector<int> &getPorts() const;
	const std::string &getName() const;
	const std::map<int, std::string> &getErrorPages() const;
	const std::map<std::string, std::string> &getLocations() const;
	const std::map<std::string, std::vector<std::string> > &getMethods() const;
	const std::string &getCgiExtension() const;
	const std::vector<std::string> &getIndexFiles() const;
	bool isAutoIndex() const;
	int getMaxBodySize() const;
	int getClientTimeout() const;
	const std::string &getLogAccessFile() const;
	const std::string &getLogErrorFile() const;
	const std::map<std::string, std::string> &getRedirects() const;
	const std::vector<std::string> &getUpstreamServers() const;
	const std::vector<std::string> &getAllowList() const;
	const std::vector<std::string> &getDenyList() const;
	const std::map<std::string, std::string> &getMimeTypes() const;

	void addPort(int port);
	void setServerName(const std::string &name);
	void addErrorPage(int errorCode, const std::string &pagePath);
	void addLocation(const std::string &uri, const std::string &path);
	void addMethod(const std::string &uri, const std::string &method);
	void setCgiExtension(const std::string &ext);
	void addIndexFile(const std::string &file);
	void setAutoIndex(bool enabled);
	void setMaxBodySize(int size);
	void setClientTimeout(int timeout);
	void setLogAccessFile(const std::string &path);
	void setLogErrorFile(const std::string &path);
	void addRedirect(const std::string &from, const std::string &to);
	void addUpstreamServer(const std::string &backend);
	void addAllowIP(const std::string &ip);
	void addDenyIP(const std::string &ip);
	void addMimeType(const std::string &extension, const std::string &mimeType);

	void printConfig() const;

};

#endif // SERVERCONFIG_HPP
