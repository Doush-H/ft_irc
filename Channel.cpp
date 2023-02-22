#include "User.hpp"
#include "Channel.hpp"
#include "ChannelBot.hpp"

Channel::Channel(std::string name, int modes) : _name(name), _topic("No topic is set"), _modes(modes), _userCount(0) {}

Channel::~Channel() {}

Channel::Channel(const Channel &copy) {
	*this = copy;
}

Channel	&Channel::operator = (const Channel &copy) {
	if (this != &copy) {
		_name = copy.getName();
		_channel_key = copy.getChannelKey();
		_topic = copy.getTopic();
		_users = copy.getUsersMap();
		_modes = copy.getModes();
		_userCount = copy.getUserCount();
	}
	return *this;
}

void	Channel::initBotHistory() {
	std::map<const User *, privilege>::iterator	it = _users.begin();
	for (; it != _users.end(); it++) {
		channelBot.addUserHistory(*it->first, it->second);
	}
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

#include <iostream>

size_t	Channel::countUsers() const
{
	return (_userCount);
}

void	Channel::setPrivilege(const User &user, privilege priv)
{
	std::map<const User *, privilege>::iterator	it = _users.find(&user);

	if (it == _users.end())
		return ;
	it->second = priv;
}

int	Channel::findUser(const User &user) const
{
	std::map<const User *, privilege>::const_iterator	it = _users.find(&user);

	if (it == _users.end())
		return (-1);
	return (it->second);
}

void	Channel::removeUser(const User &user)
{
	_users.erase(&user);
	_userCount--;
}

void	Channel::addUser(User &user, privilege priv)
{
	std::map<const User *, privilege>::iterator userIt = _users.find(&user);
	if ((userIt != _users.end() && userIt->second == INVITED && priv != INVITED) || (userIt == _users.end() && priv != INVITED))
		_userCount++;

	if (userIt == _users.end())
		_users.insert(std::pair<const User *, privilege>(&user, priv));
	else
		userIt->second = priv;
	
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

const std::map<const User *, privilege>& Channel::getUsersMap() const {
	return _users;
}

int	Channel::getModes() const {
	return _modes;
}

void Channel::setUserCount(int count) {
	_userCount = count;
}

int Channel::getUserCount() const {
	return _userCount;
}