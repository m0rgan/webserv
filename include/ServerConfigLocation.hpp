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
		std::string									_uri;
		std::string									_root;
		std::vector<std::string>					_indexFiles;
		bool										_autoIndex;
		std::map<int, std::string>					_errorPages;
		std::string									_proxyPass;
		int											_maxBodySize;
		std::vector<std::string>					_allowedMethods;
		std::map<std::string, std::string>			_cgiHandlers;
		std::map<std::string, std::string>			_headers;
		std::map<std::string, ServerConfigLocation>	_nestedLocations;
		std::pair<int, std::string>					_returnDirective;
    	bool										_hasReturnDirective;

	public:
		ServerConfigLocation();
		ServerConfigLocation(const std::string &uri);
		ServerConfigLocation(const ServerConfigLocation &src);
		ServerConfigLocation &operator=(const ServerConfigLocation &rhs);
		~ServerConfigLocation();

		void setURI(const std::string &uri);
		const std::string &getURI() const;
		void setRoot(const std::string &root);
		const std::string &getRoot() const;
		void addIndexFile(const std::string &file);
		const std::vector<std::string> &getIndexFiles() const;
		void setAutoIndex(bool enabled);
		bool getAutoIndex() const;
		void addAllowedMethod(const std::string &method);
		const std::vector<std::string> &getAllowedMethods() const;
		void addCgiHandler(const std::string &extension, const std::string &handler);
		const std::map<std::string, std::string> &getCgiHandlers() const;
		void addErrorPage(int errorCode, const std::string &pagePath);
		const std::map<int, std::string> &getErrorPages() const;
		void addHeader(const std::string &key, const std::string &value);
		const std::map<std::string, std::string> &getHeaders() const;
		void setProxyPass(const std::string &proxy);
		const std::string &getProxyPass() const;
		void addReturnDirective(int statusCode, const std::string &url);
		bool hasReturnDirective() const;
		int getReturnStatusCode() const;
		const std::string &getReturnUrl() const;
		const std::pair<int, std::string> &getReturnDirectives() const;
		void setMaxBodySize(int size);
		int getMaxBodySize() const;
		void addNestedLocation(const std::string &uri, const ServerConfigLocation &location);
		const std::map<std::string, ServerConfigLocation> &getNestedLocations() const;

		bool matchesURI(const std::string &requestURI) const;

		void printLocationConfig() const;
};

#endif
