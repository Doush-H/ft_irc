#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <map>
# include <list>
# include <string>

class	User;

enum	privilege { OPERATOR = 0, VOICE_PRIO = 1, NO_PRIO = 2, BANNED = 3, INVITED = 4 };
enum	modes { NONE = 0, SECRET = 1, PRIV = 2, MODERATED = 4, INVITE_ONLY = 8, TOPIC_RESTRICTED = 16 };

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
	const std::string					_name;
	std::string							_channel_key;
	std::string							_topic;
	std::map<const User *, privilege>	_users;
	int									_modes;
	// bool	_secret;
	// bool	_prv;
	// bool	_block_external_message;
	// bool	_moderated;
	// bool	_invite_only;
	// bool	_topic_change_restricted;
	// int	_limit;
public:
	Channel(std::string name, int modes);
	~Channel();
    //string getter setters
	const std::string	&getName() const;
	const std::string	&getChannelKey() const;
	void	setChannelKey(const std::string &key);
	const std::string	&getTopic() const;
	void	setTopic(const std::string &key);
    //user getter setters
	// void	removeAllGroups(const User &user);
	void	setPrivilege(const User &user, privilege priv);
	size_t	countUsers();
	int		findUser(const User &user);
	void	removeUser(const User &user);
	void	addUser(User &user, privilege privilege);
	bool	checkModes(int modes);
	void	setModes(int modes);
	void	removeModes(int modes);
	
};

#endif