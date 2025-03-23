/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 13:00:08 by migumore          #+#    #+#             */
/*   Updated: 2025/03/23 13:52:59 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

#include <ConfigFileServer.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <set>

const std::string DEFAULT_CONFIG = "default.conf"; //check if correct

class ConfigFile
{
	private:
		std::vector<ConfigFileServer> _parsedServers;

	public:
		ConfigFile();
		ConfigFile(const ConfigFile &src);
		ConfigFile &operator=(const ConfigFile &rhs);
		~ConfigFile();

		const std::vector<ConfigFileServer> &getServers() const;
		
		void parser(const std::string &configFile);
		ConfigFileServer parseServerBlock(std::ifstream &file);
		void parseListenDirective(const std::string &listenValue, ConfigFileServer &ConfigFileServer);
		ConfigFileServerLocation parseLocationBlock(std::ifstream &file, const std::string &locationPath);
		
		void serverParseKeyValue(std::istringstream &lineStream, const std::string &key, ConfigFileServer &ConfigFileServer);
		void locationParseKeyValue(std::istringstream &lineStream, const std::string &key, ConfigFileServerLocation &locationConfig);
		
		bool isDuplicateServer(const std::vector<ConfigFileServer> &servers, const ConfigFileServer &newServer);
		bool hasDuplicateHostPort(const ConfigFileServer &ConfigFileServer);
		size_t sizeConversion(const std::string &sizeStr);
		std::string &ignoreComments(std::string &line);

		void printConfig() const; //debug must delete
};

#endif

//can there be virtual hosts like these:

// Use Virtual Hosts (Same Port, Different Domains)
// If you're using the same port but want to run different web apps, use Virtual Hosts.

// Example nginx.conf:

// server {
//     listen 8080;
//     server_name app1.local;
//     root /var/www/app1;
// }

// server {
//     listen 8080;
//     server_name app2.local;
//     root /var/www/app2;
// }

// struct LocationConfig
// {
// 	std::string root;
// 	std::string alias;
// 	std::vector<std::string> indexFiles;
// 	std::vector<std::string> allowedMethods;
// 	std::map<int, std::string> errorPages;
// 	std::map<std::string, std::string> headers;
// 	size_t maxBodySize;
// 	bool autoIndex;
// 	std::pair<int, std::string> returnDirective;
// 	bool hasReturnDirective;
// 	std::string	uri;
// 	std::map<std::string, LocationConfig>	nestedLocations;

// 	LocationConfig() : maxBodySize(1048576), autoIndex(false), hasReturnDirective(false) {}
// };

// struct ServerConfig
// {
// 	std::string serverName; // Defines the virtual host names server_name example.com www.example.com; Only responds to requests with Host: example.com
// 	std::vector<std::pair <std::string, int> > hostPort;
// 	std::string root;
// 	std::vector<std::string> indexFiles;
// 	std::map<int, std::string> errorPages;
// 	std::map<std::string, LocationConfig> locations;
// 	size_t maxBodySize;
// 	bool autoIndex;
// 	std::pair<int, std::string> returnDirective;
// 	bool hasReturnDirective;

// 	ServerConfig() : maxBodySize(1048576), autoIndex(false), hasReturnDirective(false) {}
// };