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

//include httprequest struct in class
//include response status code? to save last code of request?
//make responses like a template in this class instead of inside member function in Client?
// HTTP response status codes

// HTTP response status codes indicate whether a specific HTTP request has been successfully completed. Responses are grouped in five classes:

// Informational responses (100 – 199)
// Successful responses (200 – 299)
// Redirection messages (300 – 399)
// Client error responses (400 – 499)
// Server error responses (500 – 599)
#endif // HTTPPARSER_HPP