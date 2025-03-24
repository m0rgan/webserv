/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EPoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/03 11:10:17 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/03 11:10:17 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "EPoll.hpp"

EPoll::EPoll() : _events(MAX_EVENTS)
{
	_epollFd = epoll_create(MAX_EVENTS);
	if (_epollFd == -1)
		throw std::runtime_error("Failed to create epoll instance");
	if (fcntl(_epollFd, F_SETFD, FD_CLOEXEC) == -1)
		throw std::runtime_error("Failed to set FD_CLOEXEC on epoll instance");
}

EPoll::~EPoll()
{
	close(_epollFd);
}

void EPoll::addFD(int fd, uint32_t events)
{
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;

	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev) == -1)
		throw std::runtime_error("Failed to add fd to epoll");

	_fdEvents[fd] = events;
}

void EPoll::modifyFD(int fd, uint32_t events)
{
	struct epoll_event ev;
	ev.events = events;
	ev.data.fd = fd;

	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &ev) == -1)
		throw std::runtime_error("Failed to modify fd in epoll");

	_fdEvents[fd] = events;
}

void EPoll::removeFD(int fd)
{
	if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
		throw std::runtime_error("Failed to remove fd from epoll");

	_fdEvents.erase(fd);
}

int EPoll::wait()
{
	return (epoll_wait(_epollFd, _events.data(), MAX_EVENTS, 0)); //TIMEOUT not neccesary?
}

struct epoll_event EPoll::getEvent(int index)
{
	return (_events[index]);
}
