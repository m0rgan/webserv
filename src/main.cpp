/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 15:02:32 by migumore          #+#    #+#             */
/*   Updated: 2025/03/26 16:30:41 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ServerLauncher.hpp>

int main(int argc, char *argv[])
{
	if (argc > 2)
		return (std::cerr << "How to use: ./webserv <config_file>" << std::endl, 1);
	std::string configFile = (argc == 2) ? argv[1]: DEFAULT_CONFIG;

	ServerLauncher webserv;
	try
	{
		webserv.initServers(configFile);
		webserv.loop();
	}
	catch (const std::exception &e)
	{
		return (std::cerr << e.what() << std::endl, 1);
	}
	return (0);
}

//GET
//curl -v http://localhost:8080
//curl -v http://localhost:9090
//POST
//curl -v -X POST -H "X-Filename: my_document.txt" -d "Hello, this is my file content." http://localhost:8080/
//DELETE
//curl -v -X DELETE http://localhost:8080/file-to-delete