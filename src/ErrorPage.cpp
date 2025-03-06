/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPage.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 19:28:29 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/28 19:28:29 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ErrorPage.hpp"

const std::map<int, std::string> ErrorPage::errorStatusCodes = ErrorPage::initErrorStatusCodes();

std::map<int, std::string> ErrorPage::initErrorStatusCodes()
{
	std::map<int, std::string> codes;
	codes[400] = "Bad Request";
	codes[401] = "Unauthorized";
	codes[403] = "Forbidden";
	codes[404] = "Not Found";
	codes[405] = "Method Not Allowed";
	codes[408] = "Request Timeout";
	codes[413] = "Payload Too Large";
	codes[500] = "Internal Server Error";
	codes[501] = "Not Implemented";
	codes[503] = "Service Unavailable";
	codes[504] = "Gateway Timeout";
	return (codes);
}

std::string ErrorPage::generate(int errorCode)
{
	std::string errorName = "Unknown Error";
	std::map<int, std::string>::const_iterator it = errorStatusCodes.find(errorCode);
	if (it != errorStatusCodes.end())
		errorName = it->second;
	std::ostringstream html;
	html << "<html><head><title>" << errorCode << " " << errorName << "</title></head>"
		<< "<body><h1>" << errorCode << " - " << errorName << "</h1>"
		<< "<p>Something went wrong. Ask migumore because gabrifer doesn't know</p></body></html>";

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
	return (errorStatusCodes.find(statusCode) != errorStatusCodes.end());
}