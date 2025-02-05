/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 14:56:40 by migumore          #+#    #+#             */
/*   Updated: 2025/02/03 15:05:13 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include <HttpRequest.hpp>
#include <iostream>
#include <sstream>

class HttpParser
{
public:
    HttpRequest parseRequest(const std::string &rawRequest);
};

#endif // HTTPPARSER_HPP

