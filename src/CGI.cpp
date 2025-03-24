/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 10:46:00 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/23 19:33:51 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI(void){};

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
		this->_envBuffer = rhs._envBuffer;
		this->_env = rhs._env;
		this->_currentConfig = rhs._currentConfig;
	}
	return (*this);
};

CGI::~CGI(void){};

CGI::CGI(ConfigFileServer const &currentConfig) : _currentConfig(currentConfig) {}

void CGI::setupEnvironment(const HTTPRequest &http)
{
	_envBuffer.reqMethod = "REQUEST_METHOD=" + http.request.method;
	_envBuffer.reqUri = "REQUEST_URI=" + http.request.uri;
	_envBuffer.pathInfo = "PATH_INFO=" + _fullPath;
	_envBuffer.scriptFilename = "SCRIPT_FILENAME=" + _fullPath;
	if (http.request.headers.count("Content-Length"))
		_envBuffer.contentLength = "CONTENT_LENGTH=" + http.request.headers.at("Content-Length");
	else
		_envBuffer.contentLength = "CONTENT_LENGTH=0";
	if (http.request.headers.count("User-Agent"))
		_envBuffer.userAgent = "HTTP_USER_AGENT=" + http.request.headers.at("User-Agent");
	if (http.request.headers.count("Host"))
		_envBuffer.host = "HTTP_HOST=" + http.request.headers.at("Host");
	if (http.request.headers.count("Referer"))
		_envBuffer.referer = "HTTP_REFERER=" + http.request.headers.at("Referer");
	if (_fullPath.find(".php") != std::string::npos)
	{
		_envBuffer.redirectStatus = "REDIRECT_STATUS=1";
		_env.push_back(const_cast<char*>(_envBuffer.redirectStatus.c_str()));
	}
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
	if (http.request.headers.find("Cookie") != http.request.headers.end())
	{
		std::string cookies = http.request.headers.at("Cookie");
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

void CGI::execute(HTTPRequest *http)
{
	_fullPath = http->resolveFilePath(_currentConfig);
	// std::cout << "CGI FILE PATH : " << _fullPath << std::endl;
	setup(*http);
	int socketPair[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, socketPair) < 0)
		throw std::runtime_error("[ERROR] Unable to create socket pair");
	(setCloexecFlag(socketPair[0]), setCloexecFlag(socketPair[1]));
	pid_t pid = fork();
	if (pid < 0)
	{
		(close(socketPair[0]), close(socketPair[1]));
		throw std::runtime_error("[ERROR] Fork failed");
	}
	if (pid == 0)
	{
		close(socketPair[0]);
		size_t pos = _fullPath.find_last_of('/');
		if (pos != std::string::npos)
		{
			std::string dirPath = _fullPath.substr(0, pos);
			if (chdir(dirPath.c_str()) == -1)
				(close(socketPair[1]), kill(0, SIGKILL));
		}
		int log_fd = open("/var/log/cgi_errors.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
		if (log_fd == -1)
			log_fd = open("/dev/null", O_WRONLY);
		if (dup2(socketPair[1], STDIN_FILENO) == -1 || dup2(socketPair[1], STDOUT_FILENO) == -1 || dup2(log_fd, STDERR_FILENO) == -1)
			(close(socketPair[1]), close(log_fd), kill(0, SIGKILL));
		if (access(_argv[0], F_OK | R_OK | X_OK) == -1)
			(close(socketPair[1]), close(log_fd), kill(0, SIGKILL));
		(close(socketPair[1]), close(log_fd));
		execve(_argv[0], _argv.data(), _env.data());
		kill(0, SIGKILL);
	}
	else
	{
		close(socketPair[1]);
		if ((http->request.method == "POST" || http->request.method == "DELETE") && !http->request.body.empty())
		{
			ssize_t bytesWritten = write(socketPair[0], http->request.body.c_str(), http->request.body.size());
			if (bytesWritten < 0)
				std::cerr << "[ERROR] writing to CGI failed" << std::endl;
		}
		// does this make sense since its dechunked? --if no Content-Length header read until EOF
		char buffer[4096];
		ssize_t bytesRead;
		while ((bytesRead = read(socketPair[0], buffer, sizeof(buffer))) > 0)
			_cgiOutput.append(buffer, bytesRead);
		if (bytesRead < 0)
			std::cerr << "[ERROR] Failed to read from CGI pipe: " << strerror(errno) << std::endl;
		
		close(socketPair[0]);
		int status;
		if (waitpid(pid, &status, 0) == -1)
			std::cerr << "[ERROR] Failed to wait for child process: " << strerror(errno) << std::endl;
		else
		{
			if (WIFEXITED(status) && WEXITSTATUS(status) == 0) // != 0 ERROR
				return; // sendErrorResponseToClient(500, "Internal Server Error");std::cerr << "CGI script exited with status: " << WEXITSTATUS(status) << std::endl;
			else if (WIFSIGNALED(status))
				std::cerr << "[ERROR] CGI script terminated by signal: " << WTERMSIG(status) << std::endl;
			else
				std::cerr << "[ERROR] CGI script exited with unknown status." << std::endl;
		}
		// returns need the serverErrorResponse validation?
	}
};

std::string const CGI::getOutput(void)
{
	return (_cgiOutput);
}