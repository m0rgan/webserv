/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManagement.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 16:42:03 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/20 16:42:03 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SessionManagement.hpp"

SessionManagement::SessionManagement(void)
{
	srand(time(NULL));
}

SessionManagement::SessionManagement(SessionManagement const &src)
{
	*this = src;
}

SessionManagement &SessionManagement::operator=(SessionManagement const &rhs)
{
	if (this != &rhs)
		_sessions = rhs._sessions;;
	return (*this);
}

SessionManagement::~SessionManagement(void) {}

std::string SessionManagement::generateSessionID(void)
{
	std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	std::string sessionID;
	do
	{
		sessionID.clear();
		for (int i = 0; i < 16; ++i)
			sessionID += chars[rand() % chars.size()];
	}
	while (_sessions.find(sessionID) != _sessions.end());
	return (sessionID);
}

std::string SessionManagement::createSession(const std::string &sessionID)
{
	std::string newSessionID = sessionID;

	if (!sessionID.empty() && _sessions.find(sessionID) != _sessions.end())
		return (sessionID);
	if (newSessionID.empty()) 
		newSessionID = generateSessionID();
	_sessions[newSessionID] = std::map<std::string, std::string>();
	return (newSessionID);
}

std::map<std::string, std::string> &SessionManagement::getSession(const std::string &sessionID)
{
	return (_sessions[sessionID]);
}

bool SessionManagement::sessionExists(const std::string &sessionID) const
{
	return (_sessions.find(sessionID) != _sessions.end());
}

void SessionManagement::destroySession(const std::string &sessionID)
{
	_sessions.erase(sessionID);
}
