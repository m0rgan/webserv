/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SessionManagement.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabrielfernandezleroux <gabrielfernande    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 16:41:13 by gabrielfern       #+#    #+#             */
/*   Updated: 2025/03/20 16:41:13 by gabrielfern      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SESSIONMANAGEMENT_HPP
#define SESSIONMANAGEMENT_HPP

#include <string>
#include <map>
#include <ctime>
#include <cstdlib>

class SessionManagement
{
	private:
		std::map<std::string, std::map<std::string, std::string> > _sessions;
		std::string generateSessionID(void);

	public:
		SessionManagement(void);
		SessionManagement(SessionManagement const &src);
		SessionManagement &operator=(SessionManagement const &rhs);
		~SessionManagement(void);

		std::string createSession(const std::string &sessionID);
		std::map<std::string, std::string> &getSession(const std::string &sessionID);
		bool sessionExists(const std::string &sessionID) const;
		void destroySession(const std::string &sessionID);
};

#endif