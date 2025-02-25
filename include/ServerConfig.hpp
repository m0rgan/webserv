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
#include <iostream>
#include "ServerConfigLocation.hpp"

class ServerConfig
{
private:
	std::vector<std::pair <std::string, int> > 	_hostPort;
	std::vector<int>							_ports; // change to pairs
	std::vector<std::string>					_hosts; // change to pairs
	// host? or just host:port
	std::string 								_name;
	// default host:port?
	std::string 								_root;
	std::map<int, std::string>					_errorPages;
	int											_maxBodySize;
	std::map<std::string, std::string>			_redirects;
	std::map<std::string, ServerConfigLocation>	_locations; // Key: URI, Value: LocationConfig
	std::string									_cgiExtension;
	std::vector<std::string>					_indexFiles;
	bool										_autoIndex;
	std::string									_logAccessFile;
	std::string									_logErrorFile;

public:
	ServerConfig(void);
	ServerConfig(const ServerConfig &src);
	ServerConfig &operator=(const ServerConfig &rhs);
	~ServerConfig(void);

	const std::vector<std::pair <std::string, int> > &getHostPort() const;
	const std::vector<int> &getPorts() const;
	const std::vector<std::string> &getHosts() const;
	const std::string &getName() const;
	const std::string &getRoot() const;
	const std::map<int, std::string> &getErrorPages() const;
	const std::map<std::string, ServerConfigLocation> &getLocations() const;
	const std::vector<std::string> &getIndexFiles() const;
	int getMaxBodySize() const;
	const std::string &getLogAccessFile() const;
	const std::string &getLogErrorFile() const;
	const std::map<std::string, std::string> &getRedirects() const;

	void addHostPort(const std::string &host, int port); // Set host:port
	void addPort(int port);
	void addHost(std::string host);
	void setServerName(const std::string &name);
	void setRoot(const std::string &root);
	void addErrorPage(int errorCode, const std::string &pagePath);
	void addLocation(const ServerConfigLocation &location); // Add location blocks
	void setCgiExtension(const std::string &ext);
	void addIndexFile(const std::string &file);
	void setAutoIndex(bool enabled);
	void setMaxBodySize(int size);
	void setLogAccessFile(const std::string &path);
	void setLogErrorFile(const std::string &path);
	void addRedirect(const std::string &from, const std::string &to);

	void printConfig() const;
};

#endif // SERVERCONFIG_HPP
