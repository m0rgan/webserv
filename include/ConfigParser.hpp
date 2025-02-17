/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 13:00:08 by migumore          #+#    #+#             */
/*   Updated: 2025/02/05 18:31:00 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <ServerConfig.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

class ConfigParser
{
private:
	std::vector<ServerConfig> servers; // Stores parsed servers

public:
	ConfigParser(const std::string &filename);
	ConfigParser(const ConfigParser &other);
	ConfigParser &operator=(const ConfigParser &other);
	~ConfigParser();

	const std::vector<ServerConfig> &getServers() const;
	
	std::vector<ServerConfig> parseConfigFile(const std::string &filename);
	std::string &ignoreComments(std::string &line);
};

#endif // CONFIGPARSER_HPP
