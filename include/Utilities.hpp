/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utilities.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/21 18:43:30 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/21 18:43:30 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <string>
#include <map>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cctype>
#include <algorithm>


#define BLUE "\033[34m"
#define GREEN "\033[32m"
#define MAGENTA "\033[35m"
#define ORANGE  "\033[38;5;214m"
#define RESET "\033[0m"

std::string		getMimeType(const std::string &extension);
std::string		getCurrentTimestamp();
unsigned long	stringTUL(const std::string &str);

#endif