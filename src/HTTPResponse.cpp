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

const std::string HTTPResponse::_endLine = "\r\n";

const std::map<int, std::string> HTTPResponse::_statusMap = HTTPResponse::initStatusMap();

std::map<int, std::string> HTTPResponse::initStatusMap()
{
	std::map<int, std::string> m;
	m[100] = "Continue";
	m[101] = "Switching Protocols";
	m[102] = "Processing";
	m[103] = "Early Hints";
	m[200] = "OK";
	m[201] = "Created";
	m[202] = "Accepted";
	m[203] = "Non-Authoritative Information";
	m[204] = "No Content";
	m[205] = "Reset Content";
	m[206] = "Partial Content";
	m[207] = "Multi-Status";
	m[208] = "Already Reported";
	m[226] = "IM Used";
	m[300] = "Multiple Choices";
	m[301] = "Moved Permanently";
	m[302] = "Found";
	m[303] = "See Other";
	m[304] = "Not Modified";
	m[305] = "Use Proxy";
	m[306] = "Switch Proxy";
	m[307] = "Temporary Redirect";
	m[308] = "Permanent Redirect";
	m[400] = "Bad Request";
	m[401] = "Unauthorized";
	m[402] = "Payment Required";
	m[403] = "Forbidden";
	m[404] = "Not Found";
	m[405] = "Method Not Allowed";
	m[406] = "Not Acceptable";
	m[407] = "Proxy Authentication Required";
	m[408] = "Request Timeout";
	m[409] = "Conflict";
	m[410] = "Gone";
	m[411] = "Length Required";
	m[412] = "Precondition Failed";
	m[413] = "Payload Too Large";
	m[414] = "URI Too Long";
	m[415] = "Unsupported Media Type";
	m[416] = "Range Not Satisfiable";
	m[417] = "Expectation Failed";
	m[418] = "I'm A Teapot";
	m[421] = "Misdirected Request";
	m[422] = "Unprocessable Entity";
	m[423] = "Locked";
	m[424] = "Failed Dependency";
	m[425] = "Too Early";
	m[426] = "Upgrade Required";
	m[428] = "Precondition Required";
	m[429] = "Too Many Requests";
	m[431] = "Request Header Fields Too Large";
	m[451] = "Unavailable For Legal Reasons";
	m[500] = "Internal Server Error";
	m[501] = "Not Implemented";
	m[502] = "Bad Gateway";
	m[503] = "Service Unavailable";
	m[504] = "Gateway Timeout";
	m[505] = "HTTP Version Not Supported";
	m[506] = "Variant Also Negotiates";
	m[507] = "Insufficient Storage";
	m[508] = "Loop Detected";
	m[510] = "Not Extended";
	m[511] = "Network Authentication Required";
	return (m);
}

HTTPResponse::HTTPResponse()
{
	_protocol = "HTTP/1.1";
	setDefaults();
}

HTTPResponse::HTTPResponse(const std::string &protocol)
{
	_protocol = protocol;
	setDefaults();
}

HTTPResponse::~HTTPResponse(){}

HTTPResponse &HTTPResponse::setStatus(int code)
{
	std::map<int, std::string>::const_iterator it = _statusMap.find(code);
	if (it == _statusMap.end())
		it = _statusMap.find(500); //not found error handle static?
	return (setStatus(it->first, it->second));
}

HTTPResponse &HTTPResponse::setStatus(int code, const std::string &reasonPhrase)
{
	std::stringstream ss;
	ss << code;
	_statusLine.first = ss.str();
	_statusLine.second = reasonPhrase;
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
			key.erase(0, key.find_first_not_of(" \t"));
			key.erase(key.find_last_not_of(" \t") + 1);
			value.erase(0, value.find_first_not_of(" \t"));
			value.erase(value.find_last_not_of(" \t") + 1);
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

HTTPResponse &HTTPResponse::setServer()
{
	setHeader("Server", "webserv/1.0"); //must set or can leave static?
	return (*this);
}

HTTPResponse &HTTPResponse::setConnection()
{
	setHeader("Connection", "Close");
	return (*this);
}

HTTPResponse &HTTPResponse::setDefaults()
{
	if (_statusLine.first.empty()) setStatus(200);
	if (_headers.find("Date") == _headers.end()) setDate();
	if (_headers.find("Server") == _headers.end()) setServer();
	if (_headers.find("Content-Length") == _headers.end()) setHeader("Content-Length", "0");
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

	response << _protocol << " " << _statusLine.first << " " << _statusLine.second << _endLine;
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		response << it->first << ": " << it->second << _endLine;
	response << _endLine;
	response << _body;
	return (response.str());
}

void HTTPResponse::logResponse(const std::string &timestamp) const
{
	std::cout << BLUE << "[" << timestamp << "] ";
	std::cout << _protocol << " " << _statusLine.first << " " << _statusLine.second << std::endl;
	// for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
	// 	std::cout << it->first << ": " << it->second << std::endl;
	// // if (!_body.empty())
	// 	std::cout << std::endl << _body << std::endl;
	std::cout << RESET;
}