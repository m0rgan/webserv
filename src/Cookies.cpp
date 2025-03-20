/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cookies.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 16:46:26 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/20 16:46:26 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cookies.hpp"

Cookies::Cookies(void) {}

Cookies::Cookies(Cookies const &src)
{
	*this = src;
}

Cookies &Cookies::operator=(Cookies const &rhs)
{
	if (this != &rhs)
		_cookies = rhs._cookies;
	return (*this);
}

Cookies::~Cookies(void) {}

void Cookies::parse(const std::string &cookieHeader)
{
	std::istringstream stream(cookieHeader);
	std::string cookie;
	while (std::getline(stream, cookie, ';'))
	{
		size_t eqPos = cookie.find('=');
		if (eqPos != std::string::npos)
		{
			std::string key = cookie.substr(0, eqPos);
			std::string value = cookie.substr(eqPos + 1);
			trimWhitespaces(key);
			trimWhitespaces(value);
			_cookies[key] = value;
		}
	}
}

std::string Cookies::getCookie(const std::string &name) const
{
	std::map<std::string, std::string>::const_iterator it = _cookies.find(name);
	if (it != _cookies.end())
		return (it->second);
	return ("");
}

void Cookies::setCookie(const std::string &name, const std::string &value)
{
	_cookies[name] = value;
}

std::string Cookies::generateSetCookieHeader() const
{
	std::string header;
	for (std::map<std::string, std::string>::const_iterator it = _cookies.begin(); it != _cookies.end(); ++it)
		header += "Set-Cookie: " + it->first + "=" + it->second + "; Path=/; HttpOnly\r\n";
	return (header);
}