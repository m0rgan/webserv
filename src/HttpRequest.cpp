/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 15:01:21 by migumore          #+#    #+#             */
/*   Updated: 2025/02/05 17:59:00 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <HttpRequest.hpp>
#include <iostream>

HttpRequest::HttpRequest()
{
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::printRequest() const
{
	std::cout << "Method: " << method << std::endl;
	std::cout << "URI: " << uri << std::endl;
	std::cout << "HTTP Version: " << httpVersion << std::endl;
	std::cout << "Headers:" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		std::cout << "  " << it->first << ": " << it->second << std::endl;
	}
	std::cout << "Body: " << body << std::endl;
}
