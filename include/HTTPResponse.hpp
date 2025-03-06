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

class HTTPResponse
{
	private:
		std::string								_protocol;
		std::pair<std::string, std::string>		_statusLine;
		std::map<std::string, std::string>		_headers;
		std::string								_body;
		static const std::map<int, std::string>	_statusMap;
		static const std::string				_endLine;

	public:
		HTTPResponse(void);
		HTTPResponse(HTTPResponse const &src);
		HTTPResponse &operator=(HTTPResponse const &rhs);
		~HTTPResponse(void);
		HTTPResponse(const std::string &protocol);
		HTTPResponse &setStatus(int code);
		HTTPResponse &setStatus(int code, const std::string &reasonPhrase);
		HTTPResponse &setHeader(const std::string &key, const std::string &value);
		HTTPResponse &setBody(const std::string &data);
		HTTPResponse &setDate();
		HTTPResponse &setServer();
		HTTPResponse &setConnection();
		HTTPResponse &setDefaults();
		std::string toString() const;
		static std::map<int, std::string> initStatusMap();
		static std::string convertTime(time_t time);
		void logResponse(const std::string &timestamp) const;
};

#endif
