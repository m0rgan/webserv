/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerLauncher.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 17:00:42 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/21 17:00:42 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERLAUNCHER_HPP
#define SERVERLAUNCHER_HPP

#include "Server.hpp"
#include "ConfigParser.hpp"
#include <vector>
#include <iostream>
#include <csignal>

class ServerLauncher
{
	private:
		std::vector<Server *> _servers;

	public:
		ServerLauncher(const std::string &configFile);
		ServerLauncher(const ServerLauncher &src);
		ServerLauncher &operator=(const ServerLauncher &rhs);
		~ServerLauncher();

		void launch();
		// void stopServers();
		// void restartServer(size_t index);
};

#endif
