/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SignalHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 11:42:58 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/15 11:42:58 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SignalHandler.hpp"

SignalHandler::SignalHandler(void){}

SignalHandler::SignalHandler(SignalHandler const &src)
{
	this->clientSocket = src.clientSocket;
	return ;
}

SignalHandler &SignalHandler::operator=(SignalHandler const &rhs)
{
	if (this != &rhs)
		this->clientSocket = rhs.clientSocket;
	return (*this);
}

SignalHandler::~SignalHandler(void){}

void SignalHandler::handleUrgentData(int sig)
{
	int		bytesRead;
	char	buffer[1];

	(void)sig;
	bytesRead = recv(clientSocket, buffer, sizeof(buffer), MSG_OOB);
	if (bytesRead > 0)
		std::cout << "Received OOB data: " << buffer[0] << std::endl; //must handle this scenario
}

int SignalHandler::clientSocket = -1; // defines clientSocket to independent object not tied to class
