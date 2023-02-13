#include "Channel.hpp"
#include "ChannelBot.hpp"
#include "User.hpp"

#include <iostream>

ChannelBot::ChannelBot() {
	_isReal = false;
	_botUser.setUserFd(-2);
	_botUser.setHostmask("127.0.0.1");
	_botUser.setNick("channelBot");
	_botUser.setFullName("Channel Botson");
	_botUser.setName("channelBot");
	pthread_mutex_init(&_enabledMutex, NULL);
}

static void	*threadStart(void *data) {
	ChannelBot	*chanBot = static_cast <ChannelBot *> (data);
	int	timer = 30;
	while (1) {
		if (timer <= 0) {
			break ;
		}
		sleep (1);
		timer--;
		if (chanBot->getBotEnabled())
			timer = 30;
		std::cout << timer << std::endl;
	}
	std::cout << "quitting bot" << std::endl;
	chanBot->setDespawnBot(true);
	pthread_exit(NULL);
}

ChannelBot::ChannelBot(Channel &chan) {
	_isReal = true;
	_botUser.setUserFd(-2);
	_botUser.setHostmask("127.0.0.1");
	_botUser.setNick("channelBot");
	_botUser.setFullName("Channel Botson");
	_botUser.setName("channelBot");
	std::map<const User *, privilege>	users = chan.getUsersMap();
	std::map<const User *, privilege>::const_iterator	it = users.begin();
	for (; it != users.end(); it++) {
		std::string	str = it->first->getNick() + "!" + it->first->getName() + "@" + it->first->getHostmask();
		_userHistory.insert(std::pair<std::string, privilege> (str, it->second));
	}
	chan.addUser(_botUser, OPERATOR);
	pthread_mutex_init(&_enabledMutex, NULL);
	pthread_create(&_botThread, NULL, &threadStart, this);
}

ChannelBot::ChannelBot(ChannelBot &copy) {
	pthread_mutex_init(&_enabledMutex, NULL);
	*this = copy;
}

ChannelBot	&ChannelBot::operator = (ChannelBot &copy) {
	if (this != &copy) {
		_botUser.setUserFd(-2);
		_botUser.setHostmask("127.0.0.1");
		_botUser.setNick("channelBot");
		_botUser.setFullName("Channel Botson");
		_botUser.setName("channelBot");
		// _botUser = copy.getBotUser();
		_botEnabled = copy.getBotEnabled();
		_despawnBot = copy.getDespawnBot();
		_isReal = copy.getIsReal();
		_userHistory = copy.getUserHistory();
	}
	return *this;
}

ChannelBot::~ChannelBot() {
	pthread_mutex_destroy(&_enabledMutex);
}

User	ChannelBot::getBotUser() const {
	return _botUser;
}

bool	ChannelBot::getBotEnabled() {
	pthread_mutex_lock(&_enabledMutex);
	bool	temp = _botEnabled;
	pthread_mutex_unlock(&_enabledMutex);
	return (temp);
}

void	ChannelBot::setBotEnabled(bool enabled) {
	pthread_mutex_lock(&_enabledMutex);
	_botEnabled = enabled;
	pthread_mutex_unlock(&_enabledMutex);
}

bool	ChannelBot::getDespawnBot() {
	pthread_mutex_lock(&_enabledMutex);
	bool	temp = _despawnBot;
	pthread_mutex_unlock(&_enabledMutex);
	return (temp);
}

void	ChannelBot::setDespawnBot(bool despawn) {
	pthread_mutex_lock(&_enabledMutex);
	_despawnBot = despawn;
	pthread_mutex_unlock(&_enabledMutex);
}

bool	ChannelBot::getIsReal() const {
	return _isReal;
}

void	ChannelBot::setIsReal(bool real) {
	_isReal = real;
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

void	ChannelBot::beginThread(Channel &chan) {
	std::map<const User *, privilege>	users = chan.getUsersMap();
	std::map<const User *, privilege>::const_iterator	it = users.begin();
	for (; it != users.end(); it++) {
		std::string	str = it->first->getNick() + "!" + it->first->getName() + "@" + it->first->getHostmask();
		_userHistory.insert(std::pair<std::string, privilege> (str, it->second));
	}
	chan.addUser(_botUser, OPERATOR);
	pthread_create(&_botThread, NULL, &threadStart, this);
}