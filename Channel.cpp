#include "Channel.hpp"
#include "User.hpp"

Channel::Channel(std::string name, int modes) : _name(name), _topic("No topic is set"), _modes(modes), _count(0), _botUser(NULL), _botEnabled(false), _botExit(false) {

	pthread_mutex_init(&_userMutex, NULL);
	pthread_mutex_init(&_exitMutex, NULL);
}

Channel::~Channel() {
	if (_botUser)
		delete _botUser;
	pthread_mutex_destroy(&_userMutex);
	pthread_mutex_destroy(&_exitMutex);
}

const std::string	&Channel::getName() const
{
	return (_name);
}

const std::string	&Channel::getChannelKey() const
{
	return (_channel_key);
}

void	Channel::setChannelKey(const std::string &key)
{
	_channel_key = key;
}

const std::string	&Channel::getTopic() const
{
	return (_topic);
}

void	Channel::setTopic(const std::string &topic)
{
	_topic = topic;
}

size_t	Channel::countUsers() const
{
	// pthread_mutex_lock(&_userMutex);
	// size_t	temp = _users.size();
	// pthread_mutex_unlock(&_userMutex);
	return (_count);
}

void	Channel::setPrivilege(const User &user, privilege priv)
{
	pthread_mutex_lock(&_userMutex);
	std::map<const User *, privilege>::iterator	it = _users.find(&user);

	if (it == _users.end()) {
		pthread_mutex_unlock(&_userMutex);
		return ;
	}
	it->second = priv;
	pthread_mutex_unlock(&_userMutex);
}

int	Channel::findUser(const User &user)
{
	pthread_mutex_lock(&_userMutex);
	std::map<const User *, privilege>::const_iterator	it = _users.find(&user);

	if (it == _users.end()) {
		pthread_mutex_unlock(&_userMutex);
		return (-1);
	}
	int	priv = it->second;
	pthread_mutex_unlock(&_userMutex);
	return (priv);
}

void	Channel::removeUser(const User &user)
{
	pthread_mutex_trylock(&_userMutex);
	_users.erase(&user);
	_count--;
	pthread_mutex_unlock(&_userMutex);
}

void	Channel::addUser(User &user, privilege priv)
{
	pthread_mutex_trylock(&_userMutex);
	if (_users.find(&user) == _users.end())
		_users.insert(std::pair<const User *, privilege>(&user, priv));
	_count++;
	pthread_mutex_unlock(&_userMutex);
}


bool	Channel::checkModes(int modes) const
{
	return modes == (_modes & modes);
}

void	Channel::setModes(int modes) 
{
	_modes |= modes;
}

void	Channel::removeModes(int modes) 
{
	_modes &= ~modes;
}

std::map<const User *, privilege>&	Channel::getUsersMap() {
	return _users;
}

pthread_mutex_t	*Channel::getUserMutex() {
	return (&_userMutex);
}

pthread_mutex_t	*Channel::getExitMutex() {
	return (&_exitMutex);
}

#include <iostream>

void	*threadStart(void *data)
{
	Channel	*chan = static_cast <Channel *> (data);
	int	timer = 60;
	while (1) {
		if (timer <= 0) {
			chan->toggleBotExit();
			chan->toggleBotEnabled();
			break ;
		}
		sleep (1);
		timer --;
		if (!chan->getBotExit())
			timer = 60;
		std::cout << timer << std::endl;
	}
	pthread_exit(NULL);
}

void	Channel::toggleBotEnabled() {
	_botEnabled = !_botEnabled;
	if (!_botEnabled) {
		toggleBotExit();
	}
}

bool	Channel::getBotEnabled() const {
	return (_botEnabled);
}

void	Channel::generateChannelBot()
{
	_botUser = new User (-1, "127.0.0.1");
	_botUser->setNick("channelBot");
	_botUser->setFullName("Channel Botson");
	_botUser->setName("channelBot");
	std::map<const User *, privilege>::iterator	it = _users.begin();
	for (; it != _users.end(); it++) {
		std::string	str = it->first->getNick() + "!" + it->first->getName() + "@" + it->first->getHostmask();
		_userHistory.insert(std::pair<std::string, privilege> (str, it->second));
	}
	addUser(*_botUser, OPERATOR);
	pthread_create(&_botThread, NULL, &threadStart, this);
}

bool	Channel::getBotExit() {
	pthread_mutex_lock(&_exitMutex);
	bool	ret = _botExit;
	pthread_mutex_unlock(&_exitMutex);
	return (ret);
}

void	Channel::toggleBotExit() {
	pthread_mutex_lock(&_exitMutex);
	_botExit = !_botExit;
	pthread_mutex_unlock(&_exitMutex);
}

privilege	Channel::checkUserHistory(User &user) {
	std::map<std::string, privilege>::iterator	it = _userHistory.begin();
	for (; it != _userHistory.end(); it++) {
		if (user.getNick() + "!" + user.getName() + "@" + user.getHostmask() == it->first) {
			return (it->second);
		}
	}
	return (NO_PRIO);
}