#include "Channel.hpp"
#include "ChannelBot.hpp"
#include "User.hpp"

#include <iostream>

static void	*threadStart(void *data) {
	ChannelBot	*chanBot = static_cast <ChannelBot *> (data);
	int	timer = 60;
	while (1) {
		if (timer <= 0) {
			break ;
		}
		sleep (1);
		timer--;
		if (chanBot->getBotEnabled())
			timer = 60;
		std::cout << timer << std::endl;
	}
	chanBot->setDespawnBot(true);
	pthread_exit(NULL);
}

ChannelBot::ChannelBot(Channel &chan) {
	_botUser = NULL;
	_botUser = new User (-1, "127.0.0.1");
	_botUser->setNick("channelBot");
	_botUser->setFullName("Channel Botson");
	_botUser->setName("channelBot");
	std::map<const User *, privilege>	users = chan.getUsersMap();
	std::map<const User *, privilege>::const_iterator	it = users.begin();
	for (; it != users.end(); it++) {
		std::string	str = it->first->getNick() + "!" + it->first->getName() + "@" + it->first->getHostmask();
		_userHistory.insert(std::pair<std::string, privilege> (str, it->second));
	}
	chan.addUser(*_botUser, OPERATOR);
	pthread_mutex_init(&_enabledMutex, NULL);
	pthread_create(&_botThread, NULL, &threadStart, this);
}

ChannelBot::~ChannelBot() {
	if (_botUser)
		delete _botUser;
	pthread_mutex_destroy(&_enabledMutex);
}

User	*ChannelBot::getBotUser() {
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

privilege	ChannelBot::checkUserHistory(User &user) {
	std::map<std::string, privilege>::iterator	it = _userHistory.begin();
	for (; it != _userHistory.end(); it++) {
		if (user.getNick() + "!" + user.getName() + "@" + user.getHostmask() == it->first) {
			return (it->second);
		}
	}
	return (NO_PRIO);
}
