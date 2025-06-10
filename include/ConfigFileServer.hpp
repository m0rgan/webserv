/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFileServer.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/23 13:42:42 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/04/02 18:19:38 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGFILESERVER_HPP
#define CONFIGFILESERVER_HPP

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <ConfigFileServerLocation.hpp>
#include <set>
#include <algorithm>
#include <stdexcept>

class ConfigFileServer
{
	private:
		std::string 									_serverName;
		std::vector<std::pair <std::string, int> > 		_hostPort;
		std::string 									_root;
		std::vector<std::string>						_indexFiles;
		std::map<int, std::string>						_errorPages;
		std::map<std::string, ConfigFileServerLocation>	_locations;
		size_t											_maxBodySize;
		bool											_autoIndex;
		std::pair<int, std::string> 					_returnDirective;
		bool 											_hasReturnDirective;

	public:
		ConfigFileServer(void);
		ConfigFileServer(const ConfigFileServer &src);
		ConfigFileServer &operator=(const ConfigFileServer &rhs);
		~ConfigFileServer(void);

		const std::vector<std::pair <std::string, int> > &getHostPort() const;
		const std::string &getServerName() const;
		const std::string &getRoot() const;
		const std::map<int, std::string> &getErrorPages() const;
		const std::map<std::string, ConfigFileServerLocation> &getLocations() const;
		const std::vector<std::string> &getIndexFiles() const;
		size_t getMaxBodySize() const;
		bool getAutoIndex() const;
		
		void addHostPort(const std::string &host, int port);
		void setServerName(const std::string &name);
		void setRoot(const std::string &root);
		void addErrorPage(int errorCode, const std::string &pagePath);
		void addLocation(const ConfigFileServerLocation &location);
		void addIndexFile(const std::string &file);
		void setIndexFiles(const std::vector<std::string> &files);
		void setAutoIndex(bool enabled);
		void setMaxBodySize(size_t size);
		void addReturnDirective(int statusCode, const std::string &url);
		bool hasReturnDirective() const;
		int getReturnStatusCode() const;
		const std::string &getReturnUrl() const;
		const std::pair<int, std::string> &getReturnDirective() const;		
		bool hasDuplicateHostPortInBlock() const;

};

#endif
