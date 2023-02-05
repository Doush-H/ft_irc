#include "Channel.hpp"
#include "User.hpp"

Channel::Channel(std::string name, int modes) : _name(name), _modes(modes), _topic("No topic is set")
{
	// _secret = false;
	// _prv = false;
	// _block_external_message = false;
	// _moderated = false;
	// _invite_only = false;
	// _topic_change_restricted = false;
	// _limit = -1;
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

void	Channel::setTopic(const std::string &topic)
{
	_topic = topic;
}

// void	Channel::removeAllGroups(const User &user)
// {
// 	removeOperator(user);
// 	removeVoicePrio(user);
// 	removeNoPrio(user);
// 	removeBanned(user);
// }

size_t	Channel::countUsers()
{
	return (_users.size());
}

void	Channel::setPrivilege(const User &user, privilege priv)
{
	std::map<const User *, privilege>::iterator	it = _users.find(&user);

	if (it == _users.end())
		return ;
	it->second = priv;
}

int	Channel::findUser(const User &user)
{
	std::map<const User *, privilege>::iterator	it = _users.find(&user);

	if (it == _users.end())
		return (-1);
	return (it->second);
}

void	Channel::removeUser(const User &user)
{
	_users.erase(&user);
}

void	Channel::addUser(User &user, privilege priv)
{
	// if (_limit != -1 && countUsers() >= _limit)
	// 	return ;
	if (_users.find(&user) == _users.end())
		_users.insert(std::pair<const User *, privilege>(&user, priv));
}

const std::string	Channel::constructWho(const User &user)
{
	std::string	ret;
	std::map<const User *, privilege>::iterator	it = _users.begin();

//constructedString += SERV_PREFIX "352 " + msg.getSenderUser().getNick() + " * " \
	+ msg.getSenderUser().getNick() + " 42irc.com * :" + msg.getSenderUser().getFullName() + "\n\r";

	ret += ":42irc.com 352 " + user.getNick() + " * " + user.getNick() \
		+ " 42irc.com * :" + user.getFullName() + "\n\r";
	for (; it != _users.end(); it++)
		if (&user != it->first && _users.size() > 0)
			ret += ":42irc.com 352 " + user.getNick() + " * " + it->first->getNick() \
				+ " 42irc.com * :" + it->first->getFullName() + "\n\r";
	ret +=  ":42irc.com 315 " + user.getNick() + " * :End of /WHO list";
	return ret;
}

bool	Channel::checkModes(int modes) 
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


// const bool	&Channel::getSecret() const
// {
// 	return (_secret);
// }

// void	Channel::setSecret(const bool to)
// {
// 	_secret = to;
// }

// const bool	&Channel::getPrivate() const
// {
// 	return (_prv);
// }

// void	Channel::setPrivate(const bool to)
// {
// 	_prv = to;
// }

// const bool	&Channel::getBlockExternal() const
// {
// 	return (_block_external_message);
// }

// void	Channel::setBlockExternal(const bool to)
// {
// 	_block_external_message = to;
// }

// const bool	&Channel::getModerated() const
// {
// 	return (_moderated);
// }

// void	Channel::setModerated(const bool to)
// {
// 	_moderated = to;
// }

// const bool	&Channel::getInviteOnly() const
// {
// 	return (_invite_only);
// }

// void	Channel::setInviteOnly(const bool to)
// {
// 	_invite_only = to;
// }

// const bool	&Channel::getTopicChangeRestricted() const
// {
// 	return (_topic_change_restricted);
// }

// void	Channel::setTopicChangeRestricted(const bool to)
// {
// 	_topic_change_restricted = to;
// }

// const int	&Channel::getLimit() const
// {
// 	return (_limit);
// }

// void	Channel::setLimit(const int to)
// {
// 	_limit = to;
// }

// const bool	&Channel::getBanHostmasks() const
// {
// 	return (_ban_hostmasks);
// }

// void	Channel::setBanHostmasks(const bool to)
// {
// 	_ban_hostmasks = to;
// }