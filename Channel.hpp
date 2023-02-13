#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <map>
# include <list>
# include <string>

class	User;
class	ChannelBot;

enum	privilege { OPERATOR = 1, VOICE_PRIO = 2, NO_PRIO = 3, INVITED = 4 };
enum	modes { NONE = 0, SECRET = 1, PRIV = 2, MODERATED = 4, INVITE_ONLY = 8, TOPIC_RESTRICTED = 16, KEY_PROTECTED = 32 };

/// @brief Channel class
/// @param _name name of the channel
/// @param _channel_key when the channel requires a key, this key has to match the users input
/// @param _topic the description of the channel
/// @param _users the users in the channel, their pointer is the key and their permission (privilege) level is the data
/// @param _modes bitmask to indicate the state (mode) of the channel, bits, starting with least significant indicate flags: -s, -p, -m, -i, -t, -k
class Channel
{
private:
	const std::string					_name;
	std::string							_channel_key;
	std::string							_topic;
	std::map<const User *, privilege>	_users;
	int									_modes;
public:
	Channel(std::string name, int modes);
	~Channel();
    //string getter setters
	ChannelBot									*channelBot;
	const std::string&							getName() const;
	const std::string&							getChannelKey() const;
	void										setChannelKey(const std::string &key);
	const std::string&							getTopic() const;
	void										setTopic(const std::string &key);
    //user getter setters
	void										setPrivilege(const User &user, privilege priv);
	size_t										countUsers() const;
	int											findUser(const User &user) const;
	void										removeUser(const User &user);
	void										addUser(User &user, privilege privilege);
	bool										checkModes(int modes) const;
	void										setModes(int modes);
	void										removeModes(int modes);
	const std::map<const User *, privilege>&	getUsersMap() const;
};

#endif