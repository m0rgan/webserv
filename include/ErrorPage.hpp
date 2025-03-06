/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPage.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 19:28:21 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/28 19:28:21 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERRORPAGE_HPP
#define ERRORPAGE_HPP

#include <fstream>
#include <sstream>
#include <string>
#include <map>

class ErrorPage
{
	private:
		static std::map<int, std::string> initErrorStatusCodes();
		static const std::map<int, std::string> errorStatusCodes;
	public:
		ErrorPage(void);
		ErrorPage(ErrorPage const &src);
		ErrorPage &operator=(ErrorPage const &rhs);
		~ErrorPage(void);
		static std::string generate(int errorCode);
		static void cleanup(const std::string &filePath);
		static bool isErrorStatusCode(int statusCode);
};

#endif
