/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 10:46:00 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/23 13:52:59 by gabrielfern      ###   ########.fr       */
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
		this->_requestBody = rhs._requestBody;
		this->_cgiProgram = rhs._cgiProgram;
		this->_argv = rhs._argv;
		this->_envBuffer = rhs._envBuffer;
		this->_env = rhs._env;
		this->_currentConfig = rhs._currentConfig;
	}
	return (*this);
};

CGI::~CGI(void){};

CGI::CGI(ConfigFileServer const &currentConfig) : _currentConfig(currentConfig) {}

unsigned long hexToULong(const std::string &hexStr)
{
	unsigned long result = 0;
	for (size_t i = 0; i < hexStr.length(); ++i)
	{
		char c = hexStr[i];
		result *= 16;
		if (c >= '0' && c <= '9')
			result += c - '0';
		else if (c >= 'a' && c <= 'f')
			result += c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
			result += c - 'A' + 10;
		else
			throw std::invalid_argument("Invalid hexadecimal character");
	}
	return (result);
}

// is each chunk terminated by a CRLF?
std::string unchunk(const std::string &chunkedBody)
{
	std::stringstream ss(chunkedBody);
	std::string unchunked;
	while (true)
	{
		std::string line;
		if (!getline(ss, line))
			break;
		if (!line.empty() && line[line.size() - 1] == '\r') // remove trailing CR
			line.erase(line.size() - 1);

		size_t chunkSize = 0;
		try
		{
			chunkSize = hexToULong(line);
		}
		catch (...) //catch all - fix to exceptions? or leave all for non managed exceptions?
		{
			break;
		}
		if (chunkSize == 0)
			break; // end of chunks
		char* buffer = new char[chunkSize];
		ss.read(buffer, chunkSize);
		unchunked.append(buffer, chunkSize);
		delete[] buffer;
		getline(ss, line); // read the trailing CRLF after the chunk
	}
	return (unchunked);
}

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

	_env.clear();
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



void CGI::parser(const HTTPRequest &http)
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

	_requestBody = http.request.body;
	std::map<std::string, std::string>::const_iterator it = http.request.headers.find("Transfer-Encoding");
	if (it != http.request.headers.end() && it->second == "chunked")
		_requestBody = unchunk(http.request.body);
}

void CGI::execute(HTTPRequest *http)
{
	_fullPath = http->resolveFilePath(_currentConfig);
	// std::cout << "CGI FILE PATH : " << _fullPath << std::endl;
	parser(*http);
	int pipe_in[2];  // for input to CGI
	int pipe_out[2]; // for output from CGI
	if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0)
	{
		std::cerr << "Error: Unable to create pipes" << std::endl;
		return;
	}
	pid_t pid = fork();
	if (pid < 0)
	{
		std::cerr << "Error: Fork failed" << std::endl;
		return;
	}
	if (pid == 0)
	{
		// change curr dir to dir containing requested file ensures any relative file paths inside the CGI script work correctly
		size_t pos = _fullPath.find_last_of('/');
		if (pos != std::string::npos)
		{
			std::string dirPath = _fullPath.substr(0, pos);
			// std::cout << dirPath.c_str() << std::endl;
			chdir(dirPath.c_str());
		}
		(dup2(pipe_in[0], STDIN_FILENO), dup2(pipe_out[1], STDOUT_FILENO), dup2(pipe_out[1], STDERR_FILENO));
		(close(pipe_in[1]), close(pipe_out[0]));
		//setcloexec???
		
		if (access(_argv[0], F_OK | R_OK | X_OK) == -1)
			std::cerr << "Error: Script not found/not readable/not executable: " << strerror(errno) << std::endl;
		execve(_argv[0], _argv.data(), _env.data());

		std::cerr << "Error: CGI execve failed" << std::endl;
		(close(pipe_in[0]), close(pipe_out[1])); //confirm this is correct?
		kill(0, SIGTERM); // will this handle exit correctly? use try catch?
	}
	else
	{
		(close(pipe_in[0]), close(pipe_out[1]));

		if ((http->request.method == "POST" || http->request.method == "DELETE") && !_requestBody.empty())
		{
			ssize_t bytesWritten = write(pipe_in[1], _requestBody.c_str(), _requestBody.size());
			if (bytesWritten < 0)
				std::cerr << "Error: writing to CGI failed" << std::endl;
			// std::cout << _requestBody.c_str() << std::endl;
		}
		close(pipe_in[1]);
		// if no Content-Length header read until EOF
		char buffer[4096];
		
		ssize_t bytesRead;
		while ((bytesRead = read(pipe_out[0], buffer, sizeof(buffer))) > 0)
			_cgiOutput.append(buffer, bytesRead);
		if (bytesRead < 0)
			std::cerr << "Error: Failed to read from CGI pipe: " << strerror(errno) << std::endl;
		close(pipe_out[0]);

		int status;
		if (waitpid(pid, &status, 0) == -1)
			std::cerr << "Error: Failed to wait for child process: " << strerror(errno) << std::endl;
		else
		{
			if (WIFEXITED(status) && WEXITSTATUS(status) == 0) // != 0 ERROR
				return; // sendErrorResponseToClient(500, "Internal Server Error");std::cerr << "CGI script exited with status: " << WEXITSTATUS(status) << std::endl;
			else if (WIFSIGNALED(status))
				std::cerr << "CGI script was terminated by signal: " << WTERMSIG(status) << std::endl;
			else
				std::cerr << "CGI script exited with unknown status." << std::endl;
		}
		// std::cout << "CGI Output:\n" << _cgiOutput << std::endl;
	}
};

std::string const CGI::getOutput(void)
{
	return (_cgiOutput);
}