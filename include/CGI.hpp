/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 10:46:11 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/23 10:46:11 by gabrielfern      ###   ########.fr       */
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

struct envCGI
{
	std::string reqMethod;
	std::string reqUri;
	std::string pathInfo;
	std::string scriptFilename;
};

class CGI
{
	private:
		std::string _fullPath;
		std::string _requestBody;
		std::string _cgiProgram;
		std::vector<char *> _argv;
		envCGI _envBuffer;
		std::vector<char *> _env;
	public:
		CGI(void);
		CGI(CGI const &src);
		CGI &operator=(CGI const &rhs);
		~CGI(void);

		bool routeToCGI(std::string requestURI);
		void execute(const HTTPRequest &http);
		void parser(const HTTPRequest &http, const std::string serverRoot);
};

#endif