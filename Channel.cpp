#include "Channel.hpp"

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

const std::string	&getTopic() const;
void	setTopic(const std::string &key);
const std::list<User>	getOperators() const;
void	removeOperator(const User &user);
void	addOperator(const User &user);
const std::list<User>	getVoicePrio() const;
void	removeVoicePrio(const User &user);
void	addVoicePrio(const User &user);
const std::list<User>	getNoPrio() const;
void	removeNoPrio(const User &user);
void	addNoPrio(const User &user);
const std::list<User>	getBanned() const;
void	removeBanned(const User &user);
void	addBanned(const User &user);
const bool	&getSecret() const;
void	setSecret(const bool to);
const bool	&getPrivate() const;
void	setPrivate(const bool to);
const bool	&getBlockExternal() const;
void	setBlockExternal(const bool to);
const bool	&getModerated() const;
void	setModerated(const bool to);
const bool	&getInviteOnly() const;
void	setInviteOnly(const bool to);
const bool	&getTopicChangeRestricted() const;
void	setTopicChangeRestricted(const bool to);
const int	&getLimit() const;
void	setLimit(const int to);
const bool	&getBanHostmasks() const;
void	setBanHostmasks(const bool to);