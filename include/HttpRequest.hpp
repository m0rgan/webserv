/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 14:54:09 by migumore          #+#    #+#             */
/*   Updated: 2025/02/05 17:59:41 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

class HttpRequest
{
private:
	std::string method;							// GET, POST, DELETE, etc.
	std::string uri;							// Request URI (e.g., "/index.html")
	std::string httpVersion;					// HTTP version (e.g., "HTTP/1.1")
	std::map<std::string, std::string> headers; // Headers (e.g., Host, Content-Type)
	std::string body;							// Request body (for POST, PUT)

public:
	HttpRequest();
	~HttpRequest();
	void printRequest() const; // Debugging function
};

#endif // HTTPREQUEST_HPP
