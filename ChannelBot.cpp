#include "Channel.hpp"
#include "ChannelBot.hpp"
#include "User.hpp"

#include <iostream>

ChannelBot::ChannelBot() {}

ChannelBot::ChannelBot(Channel &chan) {
	std::map<const User *, privilege>	users = chan.getUsersMap();
	std::map<const User *, privilege>::const_iterator	it = users.begin();
	for (; it != users.end(); it++) {
		std::string	str = it->first->getNick() + "!" + it->first->getName() + "@" + it->first->getHostmask();
		_userHistory.insert(std::pair<std::string, privilege> (str, it->second));
	}
	chan.addUser(_botUser, OPERATOR);
}

ChannelBot::ChannelBot(ChannelBot &copy) {
	*this = copy;
}

ChannelBot	&ChannelBot::operator = (ChannelBot &copy) {
	if (this != &copy) {
		// _botUser = copy.getBotUser();
		_userHistory = copy.getUserHistory();
	}
	return *this;
}

ChannelBot::~ChannelBot() {}

User	ChannelBot::getBotUser() const {
	return _botUser;
}

privilege	ChannelBot::checkUserHistory(User &user) {
	std::map<std::string, privilege>::iterator	it = _userHistory.begin();
	for (; it != _userHistory.end(); it++) {
		if (user.getNick() + "!" + user.getName() + "@" + user.getHostmask() == it->first) {
			return (it->second);
		}
	}
	return (NO_PRIO);
}

std::map<std::string, privilege>	ChannelBot::getUserHistory() const {
	return _userHistory;
}

void	ChannelBot::setIsActive(bool active) {
	_isActive = active;	
}

bool	ChannelBot::getIsActive() const {
	return _isActive;
}

struct timeval& 	ChannelBot::getTimestamp() {
	return _timestamp;
}
