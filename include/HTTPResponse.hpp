/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 19:20:45 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/04 14:06:30 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <map>
#include <string>
#include <sstream>
#include <ctime>
#include <Utilities.hpp>
#include <iostream>
#include <dirent.h>

class HTTPResponse
{
	private:
		std::string								_protocol;
		std::pair<std::string, std::string>		_statusLine;
		std::map<std::string, std::string>		_headers;
		std::string								_body;
		static const std::map<int, std::string>	_statusMap;

	public:
		HTTPResponse(void);
		HTTPResponse(HTTPResponse const &src);
		HTTPResponse &operator=(HTTPResponse const &rhs);
		~HTTPResponse(void);

		HTTPResponse &setStatus(int code);
		HTTPResponse &setHeader(const std::string &key, const std::string &value);
		HTTPResponse &setBody(const std::string &data);
		HTTPResponse &setDate();
		std::string toString() const;
		static std::map<int, std::string> initStatusMap();
		static std::string convertTime(time_t time);
		HTTPResponse &addRawHeaders(const std::string &rawHeaders);
		void logResponse(const std::string &timestamp) const;

		std::string setResponse(int statusCode, const std::string &contentType, const std::string &body, const std::string &redirectUrl = "", const std::string &additionalHeaders = "");
		static std::string directoryList(const std::string &directoryPath, const std::string &uri);
};

#endif
