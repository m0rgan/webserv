/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPage.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 19:28:29 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/04/01 17:45:54 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ErrorPage.hpp"

ErrorPage::ErrorPage(void) {}

ErrorPage::ErrorPage(ErrorPage const &src)
{
	*this = src;
}

ErrorPage &ErrorPage::operator=(ErrorPage const &rhs)
{
	if (this != &rhs)
		return (*this);
	return (*this);
}

ErrorPage::~ErrorPage(void) {}

const std::map<int, std::string> ErrorPage::_errorStatusCodes = ErrorPage::initErrorStatusCodes();

std::map<int, std::string> ErrorPage::initErrorStatusCodes()
{
	std::map<int, std::string> status;
	status[400] = "Bad Request";
	status[401] = "Unauthorized";
	status[403] = "Forbidden";
	status[404] = "Not Found";
	status[405] = "Method Not Allowed";
	status[408] = "Request Timeout";
	status[413] = "Payload Too Large";
	status[500] = "Internal Server Error";
	status[501] = "Not Implemented";
	status[503] = "Service Unavailable";
	status[504] = "Gateway Timeout";
	return (status);
}

std::string ErrorPage::generate(int errorCode)
{
	std::string errorName = "Unknown Error";
	std::map<int, std::string>::const_iterator it = _errorStatusCodes.find(errorCode);
	if (it != _errorStatusCodes.end())
		errorName = it->second;
	std::ostringstream html;
	html << "<html>\n<head><title>" << errorCode << " " << errorName << "</title></head>\n"
		 << "<body>\n<center><h1>" << errorCode << " - " << errorName << "</h1></center>\n"
		 << "<hr><center>webserv</center>\n"
		 << "<p><center>Something went wrong. Ask migumore because gabrifer doesn't know</p></center>\n</body>\n</html>\n";

	std::stringstream ss;
	ss << errorCode;

	std::string filename = "/tmp/error_" + ss.str() + ".html";
	std::ofstream file(filename.c_str());
	file << html.str();
	file.close();

	return (filename);
}

void ErrorPage::cleanup(const std::string &filePath)
{
	std::remove(filePath.c_str());
}

bool ErrorPage::isErrorStatusCode(int statusCode)
{
	return (_errorStatusCodes.find(statusCode) != _errorStatusCodes.end());
}