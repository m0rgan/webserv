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
		std::string 								_name;
		// default host:port?
		std::string 								_root;
		std::map<int, std::string>					_errorPages;
		size_t										_maxBodySize;
		std::map<std::string, ServerConfigLocation>	_locations;
		std::vector<std::string>					_indexFiles;
		bool										_autoIndex;
		std::pair<int, std::string> 				_returnDirective;
		bool 										_hasReturnDirective;

	public:
		ServerConfig(void);
		ServerConfig(const ServerConfig &src);
		ServerConfig &operator=(const ServerConfig &rhs);
		~ServerConfig(void);

		const std::vector<std::pair <std::string, int> > &getHostPort() const;
		const std::vector<int> &getPorts() const;
		const std::vector<std::string> &getHosts() const;
		const std::string &getServerName() const;
		const std::string &getRoot() const;
		const std::map<int, std::string> &getErrorPages() const;
		const std::map<std::string, ServerConfigLocation> &getLocations() const;
		const std::vector<std::string> &getIndexFiles() const;
		size_t getMaxBodySize() const;
		bool getAutoIndex() const;
		
		void addHostPort(const std::string &host, int port);
		void addPort(int port);
		void addHost(std::string host);
		void setServerName(const std::string &name);
		void setRoot(const std::string &root);
		void addErrorPage(int errorCode, const std::string &pagePath);
		void addLocation(const ServerConfigLocation &location);
		void addIndexFile(const std::string &file);
		void setAutoIndex(bool enabled);
		void setMaxBodySize(size_t size);
		void addReturnDirective(int statusCode, const std::string &url);
		bool hasReturnDirective() const;
		int getReturnStatusCode() const;
		const std::string &getReturnUrl() const;
		const std::pair<int, std::string> &getReturnDirective() const;

		//allowedMethods (getLocations is a map)
};

#endif
