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

const std::string DEFAULT_CONFIG = "default.conf";

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
