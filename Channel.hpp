#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "User.hpp"
# include <list>

//to figure out: users may be invisible, able to receive server notices, able to receive wallops, become IRC operator
// potential problem: maybe some users will be able to have low voice prio and be operators (look into)
// scenario: user is deleted, what happens if he was in one of these lists? I would have to go through all the lists and make sure he isn't one of them
// wiki article that helped me: https://en.wikipedia.org/wiki/Internet_Relay_Chat#cite_note-59 

/// @brief Channel class
/// @param _name name of the channel
/// @param _channel_key when the channel requires a key, this key has to match the users input
/// @param _topic the description of the channel
/// @param _owner the user that created the channel, **may be redundant
/// @param _operators users with privileges to ban, kick, give others privileges or change topic
/// @param _voice_prio users with voice priority
/// @param _no_prio users without voice priority, they will be unable to speak when channel is moderated
/// @param _banned users that are unallowed to join the channel
/// @param _secret this channel is secret and cannot be seen on the IRC client list
/// @param _prv this channel is private and listed in channel list as "prv" **look into
/// @param _block_external_message users cannot send messages to the channel externally
/// @param _moderated this channel is in moderated mode and members without voice priority may not send messages
/// @param _invite_only users may only join via invite
/// @param _topic_change_restricted only an operator may change the topic
/// @param _limit by default -1, otherwise will indicate the maximum amount of members, past which no more users can be added
/// @param _ban_hostmasks bans hostmasks from channel **look into
class Channel
{
private:
	const std::string	_name;
	std::string	_channel_key;
	std::string	_topic;
	std::list<User *>	_operators;
	std::list<User *>	_voice_prio;
	std::list<User *>	_no_prio;
	std::list<User *>	_banned;
	bool	_secret;
	bool	_prv;
	bool	_block_external_message;
	bool	_moderated;
	bool	_invite_only;
	bool	_topic_change_restricted;
	int	_limit;
	bool	_ban_hostmasks;
public:
	Channel(std::string name);
	~Channel();
    //string getter setters
	const std::string	&getName() const;
	const std::string	&getChannelKey() const;
	void	setChannelKey(const std::string &key);
	const std::string	&getTopic() const;
	void	setTopic(const std::string &key);
    //user getter setters
	void	removeAllGroups(const User &user);
	bool	findOperator(const User &user);
	void	removeOperator(const User &user);
	void	addOperator(User &user);
	bool	findVoicePrio(const User &user);
	void	removeVoicePrio(const User &user);
	void	addVoicePrio(User &user);
	bool	findNoPrio(const User &user);
	void	removeNoPrio(const User &user);
	void	addNoPrio(User &user);
	bool	findBanned(const User &user);
	void	removeBanned(const User &user);
	void	addBanned(User &user);
    //boolean getter setters
	//--these could potentially be differently implemented:
	const bool	&getSecret() const;
	void	setSecret(const bool to);
	const bool	&getPrivate() const;
	void	setPrivate(const bool to);
	//--my idea is to have three different lists of channels in the server class
	//--this would make for three lists: normal, secret and private channels
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
};

#endif