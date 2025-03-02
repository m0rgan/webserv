/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorPage.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 19:28:29 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/28 19:28:29 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ErrorPage.hpp"

std::string ErrorPage::generate(int errorCode)
{
	std::ostringstream html;
	html << "<html><head><title>" << errorCode << " Error</title></head>"
		<< "<body><h1>" << errorCode << " - Error</h1>"
		<< "<p>Something went wrong. Ask migumore because gabrifer doesn't know</p></body></html>";

	std::stringstream ss;
	ss << errorCode;

	std::string filename = "/tmp/error_" + ss.str() + ".html";
	std::ofstream file(filename.c_str());
	file << html.str();
	file.close();

	return (filename);
}

void ErrorPage::cleanup(const std::string &filePath)
{
	std::remove(filePath.c_str());
}
