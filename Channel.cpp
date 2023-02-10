#include "Channel.hpp"
#include "User.hpp"

Channel::Channel(std::string name, int modes) : _name(name), _topic("No topic is set"), _modes(modes), _count(1) {
	pthread_mutex_init(&_userMutex, NULL);
}

Channel::~Channel() {
	pthread_mutex_destroy(&_userMutex);
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
	pthread_mutex_lock(&_userMutex);
	_users.erase(&user);
	_count--;
	pthread_mutex_unlock(&_userMutex);
}

void	Channel::addUser(User &user, privilege priv)
{
	pthread_mutex_lock(&_userMutex);
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

#include <iostream>

void	*threadStart(void *data)
{
	Channel	*chan = static_cast <Channel *> (data);
	pthread_mutex_t	*userMutex = chan->getUserMutex();
	while (1) {
		sleep (1);
		pthread_mutex_lock(userMutex);
		if (chan->countUsers() == 3)
			std::cout << "theres 3 dudes" << std::endl;
		pthread_mutex_unlock(userMutex);
	}
	return (NULL);
}

void	Channel::generateChannelBot()
{
	pthread_create(&_botThread, NULL, &threadStart, this);
}