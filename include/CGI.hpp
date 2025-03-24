/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 10:46:11 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/23 13:52:59 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
#define CGI_HPP

#include "HTTPRequest.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <signal.h>
#include <sys/socket.h>

struct envCGI
{
	std::string reqMethod;
	std::string reqUri;
	std::string pathInfo;
	std::string scriptFilename;
	std::string contentLength;
	std::string redirectStatus;
	std::string sessionID;
	std::string userAgent;
	std::string host;
	std::string referer;
};

class CGI
{
	private:
		std::string					_fullPath;
		std::vector<char *>			_argv;
		std::vector<char *>			_env;
		envCGI						_envBuffer;
		ConfigFileServer			_currentConfig;
		std::string					_cgiOutput;

	public:
		CGI(void);
		CGI(CGI const &src);
		CGI &operator=(CGI const &rhs);
		~CGI(void);

		CGI(ConfigFileServer const &currentConfig);
		void execute(HTTPRequest *http);
		void setup(const HTTPRequest &http);
		void setupEnvironment(const HTTPRequest &http);
		std::string const getOutput(void);
};

#endif