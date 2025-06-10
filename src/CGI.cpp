/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 10:46:00 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/05/10 17:37:38 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI(void) : _serverLauncher(NULL), _done(false), _sentBody(false) {};

CGI::CGI(ServerLauncher* serverLauncher, ConfigFileServer const &currentConfig) : _serverLauncher(serverLauncher), _currentConfig(currentConfig), _done(false), _sentBody(false) {}

CGI::CGI(CGI const &src)
{
	*this = src;
};

CGI &CGI::operator=(CGI const &rhs)
{
	if (this != &rhs)
	{
		this->_fullPath = rhs._fullPath;
		this->_argv = rhs._argv;
		this->_env = rhs._env;
		this->_envBuffer = rhs._envBuffer;
		this->_currentConfig = rhs._currentConfig;
		this->_cgiOutput = rhs._cgiOutput;
		this->_cgiHeaders = rhs._cgiHeaders;
		this->_sentBody = rhs._sentBody;
		this->_done = rhs._done;
	}
	return (*this);
};

CGI::~CGI(void) {};

void CGI::setupEnvironment(const HTTPRequest &http)
{
	_envBuffer.gatewayInterface = "GATEWAY_INTERFACE=CGI/1.1";
	if (http.headers.count("Query-String"))
		_envBuffer.queryString = "QUERY_STRING=" + http.headers.at("Query-String");
	else
		_envBuffer.queryString = "QUERY_STRING=";

	if (http.headers.count("X-Forwarded-For"))
		_envBuffer.remoteAddr = "REMOTE_ADDR=" + http.headers.at("X-Forwarded-For");
	else
		_envBuffer.remoteAddr = "REMOTE_ADDR=127.0.0.1";

	if (http.headers.count("X-Server-Port"))
		_envBuffer.serverPort = "SERVER_PORT=" + http.headers.at("X-Server-Port");
	else
		_envBuffer.serverPort = "SERVER_PORT=80";

	if (http.headers.count("Protocol"))
		_envBuffer.serverProtocol = "SERVER_PROTOCOL=" + http.headers.at("Protocol");
	else
		_envBuffer.serverProtocol = "SERVER_PROTOCOL=HTTP/1.1";

	_envBuffer.reqMethod = "REQUEST_METHOD=" + http.method;
	_envBuffer.reqUri = "REQUEST_URI=" + http.uri;
	_envBuffer.pathInfo = "PATH_INFO=" + _fullPath;
	_envBuffer.scriptFilename = "SCRIPT_FILENAME=" + _fullPath;
	if (http.headers.count("Content-Length"))
		_envBuffer.contentLength = "CONTENT_LENGTH=" + http.headers.at("Content-Length");
	else
		_envBuffer.contentLength = "CONTENT_LENGTH=0";
	if (http.headers.count("User-Agent"))
		_envBuffer.userAgent = "HTTP_USER_AGENT=" + http.headers.at("User-Agent");
	if (http.headers.count("Host"))
		_envBuffer.host = "HTTP_HOST=" + http.headers.at("Host");
	if (http.headers.count("Referer"))
		_envBuffer.referer = "HTTP_REFERER=" + http.headers.at("Referer");
	if (_fullPath.find(".php") != std::string::npos)
	{
		_envBuffer.redirectStatus = "REDIRECT_STATUS=1";
		_env.push_back(const_cast<char*>(_envBuffer.redirectStatus.c_str()));
	}
	_env.push_back(const_cast<char*>(_envBuffer.gatewayInterface.c_str()));
	_env.push_back(const_cast<char*>(_envBuffer.queryString.c_str()));
	_env.push_back(const_cast<char*>(_envBuffer.remoteAddr.c_str()));
	_env.push_back(const_cast<char*>(_envBuffer.serverPort.c_str()));
	_env.push_back(const_cast<char*>(_envBuffer.serverProtocol.c_str()));
	_env.push_back(const_cast<char*>(_envBuffer.reqMethod.c_str()));
	_env.push_back(const_cast<char*>(_envBuffer.reqUri.c_str()));
	_env.push_back(const_cast<char*>(_envBuffer.pathInfo.c_str()));
	_env.push_back(const_cast<char*>(_envBuffer.scriptFilename.c_str()));
	_env.push_back(const_cast<char*>(_envBuffer.contentLength.c_str()));
	if (!_envBuffer.userAgent.empty())
		_env.push_back(const_cast<char*>(_envBuffer.userAgent.c_str()));
	if (!_envBuffer.host.empty())
		_env.push_back(const_cast<char*>(_envBuffer.host.c_str()));
	if (!_envBuffer.referer.empty())
		_env.push_back(const_cast<char*>(_envBuffer.referer.c_str()));
	_env.push_back(NULL);
}

void CGI::setup(const HTTPRequest &http)
{
	_env.clear();
	_argv.clear();
	setupEnvironment(http);
	
	std::string sessionID;
	if (http.headers.find("Cookie") != http.headers.end())
	{
		std::string cookies = http.headers.at("Cookie");
		size_t pos = cookies.find("SESSIONID=");
		if (pos != std::string::npos)
		{
			size_t end = cookies.find(";", pos);
			sessionID = cookies.substr(pos + 9, (end == std::string::npos) ? end : end - (pos + 9));
		}
	}
	if (!sessionID.empty())
	{
		_envBuffer.sessionID = "SESSIONID=" + sessionID;
		_env.push_back(const_cast<char*>(_envBuffer.sessionID.c_str()));
	}
	_argv.push_back(const_cast<char*>(_fullPath.c_str()));
	_argv.push_back(NULL);
}

void CGI::execute(HTTPRequest *http, int socketPair[2])
{
	_fullPath = http->resolvedFilePath;
	setup(*http);

	(setCloexecFlag(socketPair[0]), setCloexecFlag(socketPair[1]));
	pid_t pid = fork();
	if (pid < 0)
	{
		(close(socketPair[0]), close(socketPair[1]));
		throw std::runtime_error("[ERROR] Fork failed");
	}
	if (pid == 0)
		childProcess(socketPair, http);
	else
	{
		close(socketPair[1]);
		if ((http->method == "POST" || http->method == "DELETE") && !http->body.empty() && !_sentBody)
		{
			ssize_t bytesWritten = write(socketPair[0], http->body.c_str(), http->body.size());
			if (bytesWritten < 0)
			{
				// close(socketPair[0]);
				throw std::runtime_error("[ERROR] Writing to CGI process failed");
			}
			_sentBody = true;
		}
		if (fcntl(socketPair[0], F_SETFL, O_NONBLOCK) == -1)
		{
			close(socketPair[0]);
			throw std::runtime_error(std::string("fcntl: ") + strerror(errno));
		}
	}
		
};

void CGI::childProcess(int socketPair[2], HTTPRequest *http)
{
	close(socketPair[0]);
	size_t pos = _fullPath.find_last_of('/');
	if (pos != std::string::npos)
	{
		std::string dirPath = _fullPath.substr(0, pos);
		if (chdir(dirPath.c_str()) == -1)
		{
			(close(socketPair[1]), std::exit(1));
		}
	}
	if (dup2(socketPair[1], STDIN_FILENO) == -1 || dup2(socketPair[1], STDOUT_FILENO) == -1 || dup2(socketPair[1], STDERR_FILENO) == -1)
	{	
		(close(socketPair[1]));
		// http->~HTTPRequest();
		_serverLauncher->cleanupChild();
		// this->~CGI();
		std::exit(1);
	}
	(close(socketPair[1]));
	if (access(_argv[0], F_OK | R_OK | X_OK) == -1)
	{
		// http->~HTTPRequest();
		_serverLauncher->cleanupChild();
		// this->~CGI();
		std::exit(1);
	}

	execve(_argv[0], _argv.data(), _env.data());
	_serverLauncher->cleanupChild();
	// http->~HTTPRequest();
	std::cerr << "" <<http->headers[0] << std::endl;
	// this->~CGI();
	std::exit(1);
}

void CGI::readCGIOutput(int fd)
{

	char buffer[4096];
	ssize_t bytesRead = read(fd, buffer, sizeof(buffer));

	if (bytesRead > 0)
	{
		_cgiOutput.append(buffer, bytesRead);
	}
	else if (bytesRead == 0)
	{
		if (_cgiOutput.empty())
		{
			_done = true;
			throw std::runtime_error("[ERROR] Invalid CGI output (empty)");
		}

		extractHeadersCGIOutput();
		_done = true;
	}
	else if (bytesRead < 0)
	{
		return;
	}
}

bool CGI::isComplete() const { return _done; }
std::string const CGI::getOutput(void) { return (_cgiOutput); }
std::string const CGI::getHeaders(void) { return (_cgiHeaders); }

void CGI::extractHeadersCGIOutput(void)
{
	size_t headerEnd;
	
	if (_fullPath.find(".py") != std::string::npos)
		headerEnd = _cgiOutput.find("\n\n");
	else
		headerEnd = _cgiOutput.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		throw std::runtime_error("[ERROR] Malformed CGI output: Missing headers");

	_cgiHeaders = _cgiOutput.substr(0, headerEnd);
	_cgiOutput = _cgiOutput.substr(headerEnd + (_fullPath.find(".py") != std::string::npos ? 2 : 4));
	if (_cgiHeaders.find("Content-Type") == std::string::npos && _cgiHeaders.find("Content-type") == std::string::npos)
		throw std::runtime_error("[ERROR] Missing Content-Type in CGI output");
}
