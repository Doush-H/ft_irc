#include "Channel.hpp"
#include "User.hpp"

Channel::Channel(std::string name) : _name(name)
{
	_secret = false;
	_prv = false;
	_block_external_message = false;
	_moderated = false;
	_invite_only = false;
	_topic_change_restricted = false;
	_limit = -1;
	_ban_hostmasks = false;
}

Channel::~Channel() {}

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

void	Channel::setTopic(const std::string &key)
{
	_channel_key = key;
}

void	Channel::removeAllGroups(const User &user)
{
	removeOperator(user);
	removeVoicePrio(user);
	removeNoPrio(user);
	removeBanned(user);
}

unsigned int	Channel::countUsers()
{
	std::list<User *>::iterator	it = _operators.begin();
	unsigned int	count = 0;

	for (; it != _operators.end(); it++)
		count++;
	it = _voice_prio.begin();
	for (; it != _voice_prio.end(); it++)
		count++;
	it = _no_prio.begin();
	for (; it != _no_prio.end(); it++)
		count++;
	return (count);
}

//temp
#include <iostream>
//

bool	Channel::findOperator(const User &user)
{
	std::list<User *>::iterator	it = _operators.begin();

	for (; it != _operators.end(); it++)
	{
		if (&user == *it)
		{
			//temp
			std::cout << "found one" << std::endl;
			//
			return (true);
		}
	}
	return (false);
}

void	Channel::removeOperator(const User &user)
{
	std::list<User *>::iterator	it = _operators.begin();

	for (; it != _operators.end(); it++)
	{
		if (&user == *it)
			it = _operators.erase(it);
	}
}

void	Channel::addOperator(User &user)
{
	if (_limit != -1 && countUsers() >= _limit)
		return ;
	removeAllGroups(user);
	_operators.push_front(&user);
}

bool	Channel::findVoicePrio(const User &user)
{
	std::list<User *>::iterator	it = _voice_prio.begin();

	for (; it != _voice_prio.end(); it++)
	{
		if (&user == *it)
			return (true);
	}
	return (false);
}

void	Channel::removeVoicePrio(const User &user)
{
	std::list<User *>::iterator	it = _voice_prio.begin();

	for (; it != _voice_prio.end(); it++)
	{
		if (&user == *it)
			it = _voice_prio.erase(it);
	}
}

void	Channel::addVoicePrio(User &user)
{
	if (_limit != -1 && countUsers() >= _limit)
		return ;
	removeAllGroups(user);
	_voice_prio.push_front(&user);
}

bool	Channel::findNoPrio(const User &user)
{
	std::list<User *>::iterator	it = _no_prio.begin();

	for (; it != _no_prio.end(); it++)
	{
		if (&user == *it)
			return (true);
	}
	return (false);
}

void	Channel::removeNoPrio(const User &user)
{
	std::list<User *>::iterator	it = _no_prio.begin();

	for (; it != _no_prio.end(); it++)
	{
		if (&user == *it)
			it = _no_prio.erase(it);
	}
}

void	Channel::addNoPrio(User &user)
{
	if (_limit != -1 && countUsers() >= _limit)
		return ;
	removeAllGroups(user);
	_no_prio.push_front(&user);
}

bool	Channel::findBanned(const User &user)
{
	std::list<User *>::iterator	it = _no_prio.begin();

	for (; it != _no_prio.end(); it++)
	{
		if (&user == *it)
			return (true);
	}
	return (false);
}

void	Channel::removeBanned(const User &user)
{
	std::list<User *>::iterator	it = _banned.begin();

	for (; it != _banned.end(); it++)
	{
		if (&user == *it)
			it = _banned.erase(it);
	}
}

void	Channel::addBanned(User &user)
{
	removeAllGroups(user);
	_banned.push_front(&user);
}

const bool	&Channel::getSecret() const
{
	return (_secret);
}

void	Channel::setSecret(const bool to)
{
	_secret = to;
}

const bool	&Channel::getPrivate() const
{
	return (_prv);
}

void	Channel::setPrivate(const bool to)
{
	_prv = to;
}

const bool	&Channel::getBlockExternal() const
{
	return (_block_external_message);
}

void	Channel::setBlockExternal(const bool to)
{
	_block_external_message = to;
}

const bool	&Channel::getModerated() const
{
	return (_moderated);
}

void	Channel::setModerated(const bool to)
{
	_moderated = to;
}

const bool	&Channel::getInviteOnly() const
{
	return (_invite_only);
}

void	Channel::setInviteOnly(const bool to)
{
	_invite_only = to;
}

const bool	&Channel::getTopicChangeRestricted() const
{
	return (_topic_change_restricted);
}

void	Channel::setTopicChangeRestricted(const bool to)
{
	_topic_change_restricted = to;
}

const int	&Channel::getLimit() const
{
	return (_limit);
}

void	Channel::setLimit(const int to)
{
	_limit = to;
}

const bool	&Channel::getBanHostmasks() const
{
	return (_ban_hostmasks);
}

void	Channel::setBanHostmasks(const bool to)
{
	_ban_hostmasks = to;
}