/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 19:21:07 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/04 14:04:56 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPResponse.hpp"

HTTPResponse::HTTPResponse()
{
	_protocol = "HTTP/1.1";
	if (_statusLine.first.empty())
		setStatus(200);
	if (_headers.find("Date") == _headers.end())
		setDate();
	if (_headers.find("Server") == _headers.end())
		setHeader("Server", "webserv/1.0");
	if (_headers.find("Content-Length") == _headers.end())
		setHeader("Content-Length", "0");
}

HTTPResponse::HTTPResponse(HTTPResponse const &src) { *this = src; }

HTTPResponse &HTTPResponse::operator=(HTTPResponse const &rhs)
{
	if (this != &rhs)
	{
		_protocol = rhs._protocol;
		_statusLine = rhs._statusLine;
		_headers = rhs._headers;
		_body = rhs._body;
	}
	return (*this);
}

HTTPResponse::~HTTPResponse() {}

const std::map<int, std::string> HTTPResponse::_statusMap = HTTPResponse::initStatusMap();

std::map<int, std::string> HTTPResponse::initStatusMap()
{
	std::map<int, std::string> status;
	status[100] = "Continue";
	status[101] = "Switching Protocols";
	status[102] = "Processing";
	status[103] = "Early Hints";
	status[200] = "OK";
	status[201] = "Created";
	status[202] = "Accepted";
	status[203] = "Non-Authoritative Information";
	status[204] = "No Content";
	status[205] = "Reset Content";
	status[206] = "Partial Content";
	status[207] = "Multi-Status";
	status[208] = "Already Reported";
	status[226] = "IM Used";
	status[300] = "Multiple Choices";
	status[301] = "Moved Permanently";
	status[302] = "Found";
	status[303] = "See Other";
	status[304] = "Not Modified";
	status[305] = "Use Proxy";
	status[306] = "Switch Proxy";
	status[307] = "Temporary Redirect";
	status[308] = "Permanent Redirect";
	status[400] = "Bad Request";
	status[401] = "Unauthorized";
	status[402] = "Payment Required";
	status[403] = "Forbidden";
	status[404] = "Not Found";
	status[405] = "Method Not Allowed";
	status[406] = "Not Acceptable";
	status[407] = "Proxy Authentication Required";
	status[408] = "Request Timeout";
	status[409] = "Conflict";
	status[410] = "Gone";
	status[411] = "Length Required";
	status[412] = "Precondition Failed";
	status[413] = "Payload Too Large";
	status[414] = "URI Too Long";
	status[415] = "Unsupported Media Type";
	status[416] = "Range Not Satisfiable";
	status[417] = "Expectation Failed";
	status[418] = "I'm A Teapot";
	status[421] = "Misdirected Request";
	status[422] = "Unprocessable Entity";
	status[423] = "Locked";
	status[424] = "Failed Dependency";
	status[425] = "Too Early";
	status[426] = "Upgrade Required";
	status[428] = "Precondition Required";
	status[429] = "Too Many Requests";
	status[431] = "Request Header Fields Too Large";
	status[451] = "Unavailable For Legal Reasons";
	status[500] = "Internal Server Error";
	status[501] = "Not Implemented";
	status[502] = "Bad Gateway";
	status[503] = "Service Unavailable";
	status[504] = "Gateway Timeout";
	status[505] = "HTTP Version Not Supported";
	status[506] = "Variant Also Negotiates";
	status[507] = "Insufficient Storage";
	status[508] = "Loop Detected";
	status[510] = "Not Extended";
	status[511] = "Network Authentication Required";
	return (status);
}

HTTPResponse &HTTPResponse::setStatus(int code)
{
	std::map<int, std::string>::const_iterator it = _statusMap.find(code);
	if (it == _statusMap.end())
		it = _statusMap.find(500); //not found error handle static?
	std::stringstream ss;
	ss << code;
	_statusLine.first = ss.str();
	_statusLine.second = it->second;
	return (*this);
}

HTTPResponse &HTTPResponse::setHeader(const std::string &key, const std::string &value)
{
	_headers[key] = value;
	return (*this);
}

HTTPResponse &HTTPResponse::addRawHeaders(const std::string &rawHeaders)
{
	std::istringstream stream(rawHeaders);
	std::string line;
	while (std::getline(stream, line))
	{
		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos)
		{
			std::string key = line.substr(0, colonPos);
			std::string value = line.substr(colonPos + 1);
			trimWhitespaces(key);
			trimWhitespaces(value);
			_headers[key] = value;
		}
	}
	return (*this);
}

HTTPResponse &HTTPResponse::setBody(const std::string &data)
{
	_body = data;
	std::stringstream ss;
	ss << data.size();
	setHeader("Content-Length", ss.str());
	return (*this);
}

HTTPResponse &HTTPResponse::setDate()
{
	time_t now = time(0);
	setHeader("Date", convertTime(now));
	return (*this);
}

std::string HTTPResponse::convertTime(time_t time)
{
	tm *timeStruct = std::gmtime(&time);
	const char *days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	
	std::ostringstream oss;
	oss << days[timeStruct->tm_wday] << ", "
		<< (timeStruct->tm_mday < 10 ? "0" : "") << timeStruct->tm_mday << " "
		<< months[timeStruct->tm_mon] << " "
		<< (timeStruct->tm_year + 1900) << " "
		<< (timeStruct->tm_hour < 10 ? "0" : "") << timeStruct->tm_hour << ":"
		<< (timeStruct->tm_min < 10 ? "0" : "") << timeStruct->tm_min << ":"
		<< (timeStruct->tm_sec < 10 ? "0" : "") << timeStruct->tm_sec << " GMT";
	
	return (oss.str());
}

std::string HTTPResponse::toString() const
{
	std::ostringstream response;

	response << _protocol << " " << _statusLine.first << " " << _statusLine.second << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		response << it->first << ": " << it->second << "\r\n";
	response << "\r\n";
	response << _body;
	return (response.str());
}

void HTTPResponse::setResponse(int statusCode, const std::string &contentType, const std::string &body, const std::string &redirectUrl, const std::string &additionalHeaders)
{
	std::stringstream ss;
	ss << body.size();
	setStatus(statusCode)
		.setHeader("Content-Type", contentType)
		.setHeader("Content-Length", ss.str())
		.setBody(body);

	if (!redirectUrl.empty())
		setHeader("Location", redirectUrl).setHeader("Connection", "close");

	if (!additionalHeaders.empty())
		addRawHeaders(additionalHeaders);
}

std::string HTTPResponse::directoryList(const std::string &directoryPath, const std::string &uri)
{
	std::stringstream html;
	html << "<!DOCTYPE html>";
	html << "<html><head><title>Index of " << uri << "</title>";
	html << "<style>";
	html << "body { font-family: Arial, sans-serif; background-color: #f0f8ff; color: #000080; margin: 0; padding: 0; }";
	html << "h1 { background-color: #4682b4; color: white; padding: 10px; margin: 0; }";
	html << "ul { list-style-type: none; padding: 0; margin: 0; }";
	html << "li { padding: 8px 10px; border-bottom: 1px solid #dcdcdc; }";
	html << "li:nth-child(odd) { background-color: #e6f2ff; }";
	html << "li:nth-child(even) { background-color: #ffffff; }";
	html << "a { text-decoration: none; color: #000080; font-weight: bold; }";
	html << "a:hover { color: #4682b4; }";
	html << "</style>";
	html << "</head><body>";
	html << "<h1>Index of " << uri << "</h1><hr><ul>";

	DIR *dir = opendir(directoryPath.c_str());
	if (dir)
	{
		struct dirent *entry;
		while ((entry = readdir(dir)) != NULL)
		{
			std::string name = entry->d_name;
			if (name == ".")
				continue; // Skip current directory
			if (name == "..")
				html << "<li><a href=\"" << uri << "../\">Parent Directory</a></li>";
			else
				html << "<li><a href=\"" << uri << name << (entry->d_type == DT_DIR ? "/" : "") << "\">" << name << "</a></li>";
		}
		closedir(dir);
	}
	html << "</ul><hr></body></html>";
	return (html.str());
}

void HTTPResponse::logResponse(const std::string &timestamp) const
{
	std::cout << BLUE << "[" << timestamp << "] ";
	std::cout << _protocol << " " << _statusLine.first << " " << _statusLine.second << std::endl;
	// for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
	// 	std::cout << it->first << ": " << it->second << std::endl;
	// if (!_body.empty())
	// 	std::cout << std::endl << _body << std::endl;
	std::cout << RESET;
}