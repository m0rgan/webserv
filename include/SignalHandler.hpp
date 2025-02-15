/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SignalHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 11:40:07 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/15 11:40:07 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIGNALHANDLER_HPP
#define SIGNALHANDLER_HPP

#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

class SignalHandler
{
	public:
		static int clientSocket;

		SignalHandler(void);
		SignalHandler(SignalHandler const &src);
		SignalHandler &operator=(SignalHandler const &rhs);
		~SignalHandler(void);

		static void handleUrgentData(int sig);
};

#endif