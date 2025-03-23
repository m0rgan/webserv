/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFileServerLocation.hpp                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/23 13:42:04 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/23 13:42:04 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGFILESERVERLOCATION_HPP
#define CONFIGFILESERVERLOCATION_HPP

#include <string>
#include <vector>
#include <map>
#include <iostream>

class ConfigFileServerLocation
{
	private:
		std::string									_root;
		std::string									_alias;
		std::vector<std::string>					_indexFiles;
		std::vector<std::string>					_allowedMethods;
		std::map<int, std::string>					_errorPages;
		std::map<std::string, std::string>			_headers;
		std::string									_uri;
		bool										_autoIndex;
		size_t										_maxBodySize;
		std::pair<int, std::string>					_returnDirective;
		bool										_hasReturnDirective;
		std::map<std::string, ConfigFileServerLocation>	_nestedLocations;

	public:
		ConfigFileServerLocation();
		ConfigFileServerLocation(const ConfigFileServerLocation &src);
		ConfigFileServerLocation &operator=(const ConfigFileServerLocation &rhs);
		~ConfigFileServerLocation();
		
		ConfigFileServerLocation(const std::string &uri);
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
		void addErrorPage(int errorCode, const std::string &pagePath);
		const std::map<int, std::string> &getErrorPages() const;
		void addHeader(const std::string &key, const std::string &value);
		const std::map<std::string, std::string> &getHeaders() const;
		void addReturnDirective(int statusCode, const std::string &url);
		bool hasReturnDirective() const;
		int getReturnStatusCode() const;
		const std::string &getReturnUrl() const;
		const std::pair<int, std::string> &getReturnDirectives() const;
		void setMaxBodySize(size_t size);
		size_t getMaxBodySize() const;
		void addNestedLocation(const std::string &uri, const ConfigFileServerLocation &location);
		const std::map<std::string, ConfigFileServerLocation> &getNestedLocations() const;
		void setAlias(const std::string &alias);
		const std::string &getAlias() const;

		bool matchesURI(const std::string &requestURI) const;

		void printLocationConfig() const;
};

#endif
