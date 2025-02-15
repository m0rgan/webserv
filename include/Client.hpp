/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 16:55:28 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/02/15 17:46:22 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>

class Client
{
	private:
		int			_socket;
		std::string	_request;
		std::string	_response;

		void handleGET();
		void handlePOST();
		void handleDELETE();

	public:
		Client(void);
		Client(int socket);
		Client(Client const &src);
		Client &operator=(Client const &rhs);
		~Client(void);

		void readRequest();
		void processRequest();
		void sendResponse();
};

#endif