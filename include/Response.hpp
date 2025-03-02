/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 19:20:45 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/02 19:09:56 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <map>
#include <string>
#include <sstream>
#include <ctime>
#include <iostream>

class Response
{
	private:
		std::string								_protocol;
		std::pair<std::string, std::string>		_statusLine;
		std::map<std::string, std::string>		_headers;
		std::string								_body;
		static const std::map<int, std::string>	_statusMap;
		static const std::string				_endLine;

	public:
		Response(void);
		Response(Response const &src);
		Response &operator=(Response const &rhs);
		~Response(void);
		Response(const std::string &protocol);
		Response &setStatus(int code);
		Response &setStatus(int code, const std::string &reasonPhrase);
		Response &setHeader(const std::string &key, const std::string &value);
		Response &setBody(const std::string &data);
		Response &setDate();
		Response &setServer();
		Response &setConnection();
		Response &setDefaults();
		std::string buildResponse() const;
		static std::map<int, std::string> initStatusMap();
		static std::string convertTime(time_t time);
};

#endif
