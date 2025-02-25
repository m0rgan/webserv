/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 13:00:08 by migumore          #+#    #+#             */
/*   Updated: 2025/02/25 14:53:48 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGFILE_HPP
#define CONFIGFILE_HPP

#include <ServerConfig.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

const std::string DEFAULT_CONFIG = "default.conf"; //check if correct

class ConfigFile
{
private:
	std::vector<ServerConfig> _serverBlockConfig; // Stores parsed servers

public:
	ConfigFile(const std::string &configFile);
	ConfigFile(const ConfigFile &other);
	ConfigFile &operator=(const ConfigFile &other);
	~ConfigFile();

	const std::vector<ServerConfig> &getServers() const;
	
	std::vector<ServerConfig> parser(const std::string &configFile);
	ServerConfig parseServerBlock(std::ifstream &file);
	bool isDuplicateServer(const std::vector<ServerConfig> &servers, const ServerConfig &newServer);
	void serverParseKeyValue(std::istringstream &lineStream, const std::string &key, ServerConfig &serverConfig);
	void parseListenDirective(const std::string &listenValue, ServerConfig &serverConfig);
	ServerConfigLocation parseLocationBlock(std::ifstream &file, const std::string &locationPath);
	void locationParseKeyValue(std::istringstream &lineStream, const std::string &key, ServerConfigLocation &locationConfig);

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