/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cookies.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 16:46:01 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/20 16:46:01 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COOKIES_HPP
#define COOKIES_HPP

#include <string>
#include <map>
#include <sstream>
#include <Utilities.hpp>

class Cookies
{
	private:
		std::map<std::string, std::string> _cookies;

	public:
		Cookies(void);
		Cookies(Cookies const &src);
		Cookies &operator=(Cookies const &rhs);
		~Cookies(void);

		void parse(const std::string &cookieHeader);
		std::string getCookie(const std::string &name) const;
		void setCookie(const std::string &name, const std::string &value);
		std::string generateSetCookieHeader() const;
};

#endif