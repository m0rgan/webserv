/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfigLocation.hpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 15:08:44 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/25 19:15:12 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIGLOCATION_HPP
#define SERVERCONFIGLOCATION_HPP

#include <string>
#include <vector>
#include <map>
#include <iostream>

class ServerConfigLocation
{
private:
	std::string uri;  // The location URI (e.g., "/api", "/images")
	std::string root; // Root directory for this location
	bool autoIndex;
	int maxBodySize;
	std::map<int, std::string> errorPages;
	std::vector<std::string> allowedMethods;
	std::string redirection;
	std::vector<std::string> indexFiles;
	std::map<std::string, std::string> cgiHandlers;
	std::map<std::string, std::string> headers;
	std::string proxyPass;

public:
	ServerConfigLocation();  // Default Constructor
	ServerConfigLocation(const std::string &uri);
	ServerConfigLocation(const ServerConfigLocation &other);
	ServerConfigLocation &operator=(const ServerConfigLocation &other);
	~ServerConfigLocation();

	void setURI(const std::string &uri);
	const std::string &getURI() const;
	void setRoot(const std::string &root);
	const std::string &getRoot() const;
	void addAllowedMethod(const std::string &method);
	void addIndexFile(const std::string &file);
	const std::vector<std::string> &getIndexFiles() const;
	const std::vector<std::string> &getTryFiles() const;
	void setAutoIndex(bool enabled);
	bool getAutoIndex() const;
	void addCgiHandler(const std::string &extension, const std::string &handler);
	void addErrorPage(int errorCode, const std::string &pagePath);
	void addHeader(const std::string &key, const std::string &value);
	void setProxyPass(const std::string &proxy);
	const std::string &getProxyPass() const;
	
	void setRedirection(const std::string &to);
	void setMaxBodySize(int size);
	
	void printLocationConfig() const;
};

#endif
