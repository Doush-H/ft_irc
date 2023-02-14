#include "Channel.hpp"
#include "ChannelBot.hpp"
#include "User.hpp"

#include <iostream>

ChannelBot::ChannelBot() {
	_botUser.setFullName("Bot Chad");
	_botUser.setNick("MisterBot");
	_botUser.setName("MisterBot");
	_botUser.setUserFd(-2);
	_isActive = false;
}

// ChannelBot::ChannelBot(Channel &chan) {
// 	std::map<const User *, privilege>	users = chan.getUsersMap();
// 	std::map<const User *, privilege>::const_iterator	it = users.begin();
// 	for (; it != users.end(); it++) {
// 		std::string	str = it->first->getNick() + "!" + it->first->getName() + "@" + it->first->getHostmask();
// 		_userHistory.insert(std::pair<std::string, privilege> (str, it->second));
// 	}
// 	chan.addUser(_botUser, OPERATOR);
// }

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

User&	ChannelBot::getBotUser() {
	return _botUser;
}

privilege	ChannelBot::checkUserHistory(User &user) {
	std::map<std::string, privilege>::iterator	it = _userHistory.find(user.getNick() + "!" + user.getName() + "@" + user.getHostmask());
	if (it != _userHistory.end())
		return it->second;
	return (NO_PRIO);
}

void	ChannelBot::addUserHistory(const User &user, privilege priv) {
	std::string	hostmask = user.getNick() + "!" + user.getName() + "@" + user.getHostmask();
	std::map<std::string, privilege>::iterator	it = _userHistory.find(hostmask);
	if (it != _userHistory.end()) {
		it->second = priv;
	} else {
		_userHistory.insert(std::pair<std::string, privilege> (hostmask, priv));
	}
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
