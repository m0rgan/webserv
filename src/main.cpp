/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migumore <migumore@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 15:02:32 by migumore          #+#    #+#             */
/*   Updated: 2025/02/04 11:11:44 by migumore         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <HttpParser.hpp>
#include <iostream>

int main() {
    std::string rawRequest =
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: curl/7.68.0\r\n"
        "Content-Length: 11\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "Hello World";

    HttpParser parser;
    HttpRequest request = parser.parseRequest(rawRequest);

    std::cout << "=== Parsed HTTP Request ===" << std::endl;
    request.printRequest();

    return 0;
}
