/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 13:00:08 by migumore          #+#    #+#             */
/*   Updated: 2025/02/05 13:02:23 by migumore         ###   ########.fr       */
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
public:
    std::vector<ServerConfig> parseConfigFile(const std::string &filename);
};

#endif // CONFIGPARSER_HPP
