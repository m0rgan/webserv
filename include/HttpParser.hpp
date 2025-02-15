#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include <string>
#include <sstream>
#include <unordered_map>

struct HttpRequest
{
	std::string method;
	std::string uri;
	std::string httpVersion;
	std::unordered_map<std::string, std::string> headers;
	std::string body;
};

class HttpParser
{
	public:
		HttpRequest parseRequest(const std::string &rawRequest);
};

#endif // HTTPPARSER_HPP