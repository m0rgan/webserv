/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EPoll.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/03 11:10:36 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/03 11:10:36 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLL_HPP
#define EPOLL_HPP

#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>
#include <map>
#include <unistd.h>
#include <iostream>

#define MAX_EVENTS 4096

class EPoll
{
	private:
		int								_epollFd;
		std::vector<struct epoll_event>	_events;
		std::map<int, uint32_t>			_fdEvents;

	public:
		EPoll();
		~EPoll();

		void addFD(int fd, uint32_t events);
		void modifyFD(int fd, uint32_t events);
		void removeFD(int fd);
		int wait();
		struct epoll_event getEvent(int index);
};

#endif
