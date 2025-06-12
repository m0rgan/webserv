/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 10:46:11 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/06/10 18:30:31 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
#define CGI_HPP

#include <HTTPRequest.hpp>
#include <Client.hpp>
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
	std::string gatewayInterface;
	std::string queryString;
	std::string remoteAddr;
	std::string serverPort;
	std::string serverProtocol;
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
	std::string httpCookie;
};

class ServerLauncher;

class CGI
{
	private:
		std::string					_fullPath;
		std::vector<char *>			_argv;
		std::vector<char *>			_env;
		envCGI						_envBuffer;
		ServerLauncher*				_serverLauncher;
		ConfigFileServer			_currentConfig;
		std::string					_cgiOutput;
		std::string					_cgiHeaders;

		void setup(const HTTPRequest &http);
		void setupEnvironment(const HTTPRequest &http);
		void childProcess(int socketPair[2], HTTPRequest *http);
		void extractHeadersCGIOutput(void);

		bool _done;
		bool _sentBody;

	public:
		CGI(void);
		CGI(ServerLauncher* serverLauncher, ConfigFileServer const &currentConfig);
		CGI(CGI const &src);
		CGI &operator=(CGI const &rhs);
		~CGI(void);
		
		void 				execute(HTTPRequest *http, int socketPair[2]);
		std::string const	getOutput(void);
		std::string const	getHeaders(void);
		void 				readCGIOutput(int fd);
		bool 				isComplete() const;
};
#include <ServerLauncher.hpp>
		
#endif