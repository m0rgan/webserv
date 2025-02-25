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

class Utilities
{
	public:
		Utilities();
		Utilities(const Utilities &src);
		Utilities &operator=(const Utilities &rhs);
		~Utilities();

		std::string getMimeType(const std::string &extension) const;
};

#endif