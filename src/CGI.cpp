/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 10:46:00 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/23 10:46:00 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI(void){};

CGI::CGI(CGI const &src)
{
	(void)src;
};

CGI &CGI::operator=(CGI const &rhs)
{
	(void)rhs;
	return (*this);
};

CGI::~CGI(void){};

bool CGI::routeToCGI(std::string requestURI)
{
	size_t dotPos = requestURI.find_last_of('.');
	if (dotPos == std::string::npos)
		return (false);
	std::string extension = requestURI.substr(dotPos);
	//manage the extension being in upper/lowercase??

	//check for extensions as parameter to decide true return, is this how nginx work?
	if (extension == ".php")
		return (true);
	return (false);
};

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
		if (!line.empty() && line.back() == '\r') // remove trailing CR
			line.pop_back();

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


void CGI::parser(const HTTPRequest &http, const std::string serverRoot)
{
	size_t extPos = http.request.uri.find_last_of('.');
	if (extPos == std::string::npos || http.request.uri.substr(extPos) != ".php")
		return; // not php - must make dynamic for other extensions
	_fullPath = serverRoot + http.request.uri; // use as PATH_INFO macro, must revise
	// PATH_INFO and SCRIPT_FILENAME use full path so CGI knows which file to execute
	_envBuffer.reqMethod = "REQUEST_METHOD=" + http.request.method;
	_envBuffer.reqUri = "REQUEST_URI=" + http.request.uri;
	_envBuffer.pathInfo = "PATH_INFO=" + _fullPath;
	_envBuffer.scriptFilename = "SCRIPT_FILENAME=" + _fullPath;
	// more variables are needed?

	_env.clear();
	_env.push_back(const_cast<char*>(_envBuffer.reqMethod.c_str()));
	_env.push_back(const_cast<char*>(_envBuffer.reqUri.c_str()));
	_env.push_back(const_cast<char*>(_envBuffer.pathInfo.c_str()));
	_env.push_back(const_cast<char*>(_envBuffer.scriptFilename.c_str()));
	_env.push_back(NULL);
	_argv.clear();
	_argv.push_back(const_cast<char*>(_cgiProgram.c_str()));
	_argv.push_back(const_cast<char*>(_fullPath.c_str())); //how do it make first argument the file????
	_argv.push_back(NULL);

	// unchunk body so CGI gets EOF as end of input
	_requestBody = http.request.body;
	std::unordered_map<std::string, std::string>::const_iterator it = http.request.headers.find("Transfer-Encoding");
	if (it != http.request.headers.end() && it->second == "chunked")
		_requestBody = unchunk(http.request.body);
}

void CGI::execute(const HTTPRequest &http)
{
	const std::string serverRoot = "/var/www";              // must change to dynamic decision
	const std::string cgiProgram = "/usr/bin/php-cgi";    // must change to dynamic decision
	parser(http, serverRoot);
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
			chdir(dirPath.c_str());
		}
		(dup2(pipe_in[0], STDIN_FILENO), dup2(pipe_out[1], STDOUT_FILENO));
		(close(pipe_in[1]), close(pipe_out[0]));
		execve(cgiProgram.c_str(), _argv.data(), _env.data());
		std::cerr << "Error: CGI execve failed" << std::endl;
		kill(0, SIGTERM); // will this handle exit correctly?
	}
	else
	{
		(close(pipe_in[0]), close(pipe_out[1]));

		// if request is POST, write body to CGI reading pipe end
		if (http.request.method == "POST" && !_requestBody.empty())
		{
			ssize_t bytesWritten = write(pipe_in[1], _requestBody.c_str(), _requestBody.size());
			// change write to send?
			if (bytesWritten < 0)
				std::cerr << "Error: writing to CGI failed" << std::endl;
		}
		close(pipe_in[1]);

		// read the CGI pipe output
		// change to recv?
		// if no Content-Length header read until EOF
		char buffer[4096];
		std::string cgiOutput;
		ssize_t bytesRead;
		while ((bytesRead = read(pipe_out[0], buffer, sizeof(buffer))) > 0)
			cgiOutput.append(buffer, bytesRead);
		close(pipe_out[0]);

		int status;
		waitpid(pid, &status, 0);
		std::cout << "CGI Output:\n" << cgiOutput << std::endl; //debug to check output
	}
};
