#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <map>
# include <list>
# include <string>

# include "ChannelBot.hpp"
# include "enums.hpp"

class	User;
class	ChannelBot;

/// @brief Channel class
/// @param _name name of the channel
/// @param _channel_key when the channel requires a key, this key has to match the users input
/// @param _topic the description of the channel
/// @param _users the users in the channel, their pointer is the key and their permission (privilege) level is the data
/// @param _modes bitmask to indicate the state (mode) of the channel, bits, starting with least significant indicate flags: -s, -p, -m, -i, -t, -k
class Channel
{
private:
	std::string							_name;
	std::string							_channel_key;
	std::string							_topic;
	std::map<const User *, privilege>	_users;
	int									_modes;
public:
	Channel(std::string name, int modes);
	Channel(const Channel &copy);
	Channel	&operator = (const Channel &copy);
	~Channel();
    //string getter setters
	ChannelBot									channelBot;
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
	int											getModes() const ;
	void										removeModes(int modes);
	const std::map<const User *, privilege>&	getUsersMap() const;
};


#endif