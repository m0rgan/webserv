/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 11:08:29 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/23 11:21:54 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <iostream>
#include <sys/stat.h>
#include <Utilities.hpp>
#include <ServerConfig.hpp>

struct HttpRequest
{
	std::string method;
	std::string uri;
	std::string httpVersion;
	std::map<std::string, std::string> headers;
	std::string body;
};

// #ifndef _NGX_HTTP_REQUEST_H_INCLUDED_
// #define _NGX_HTTP_REQUEST_H_INCLUDED_

// #define NGX_HTTP_VERSION_10                1000
// #define NGX_HTTP_VERSION_11                1001

// #define NGX_HTTP_GET                       0x00000002
// #define NGX_HTTP_POST                      0x00000008
// #define NGX_HTTP_DELETE                    0x00000020

// #define NGX_HTTP_CONNECTION_CLOSE          1
// #define NGX_HTTP_CONNECTION_KEEP_ALIVE     2

// #define NGX_HTTP_OK                        200
// #define NGX_HTTP_CREATED                   201
// #define NGX_HTTP_NO_CONTENT                204
// #define NGX_HTTP_MOVED_PERMANENTLY         301
// #define NGX_HTTP_FOUND                     302
// #define NGX_HTTP_NOT_MODIFIED              304
// #define NGX_HTTP_BAD_REQUEST               400
// #define NGX_HTTP_FORBIDDEN                 403
// #define NGX_HTTP_NOT_FOUND                 404
// #define NGX_HTTP_NOT_ALLOWED               405
// #define NGX_HTTP_REQUEST_TIME_OUT          408
// #define NGX_HTTP_REQUEST_ENTITY_TOO_LARGE  413
// #define NGX_HTTP_INTERNAL_SERVER_ERROR     500
// #define NGX_HTTP_NOT_IMPLEMENTED           501
// #define NGX_HTTP_SERVICE_UNAVAILABLE       503
// #define NGX_HTTP_GATEWAY_TIME_OUT          504

// typedef struct {

//     char*                             query_string;
//     char*                             content_type;
//     unsigned long                     content_length;
    
//     unsigned                          keepalive:1;
//     unsigned                          header_only:1;
    
// } ngx_http_request_t;

// #endif /* _NGX_HTTP_REQUEST_H_INCLUDED_ */


class HTTPRequest
{
	public:
		HTTPRequest(void);
		HTTPRequest(HTTPRequest const &src);
		HTTPRequest operator=(HTTPRequest const &rhs);
		~HTTPRequest(void);

		void parser(const std::string &rawRequest);
		HttpRequest request;
		std::string resolveFilePath(const ServerConfig &config) const;
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
#endif