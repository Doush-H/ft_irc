#ifndef CHANNELBOT_HPP
# define CHANNELBOT_HPP

# include "Channel.hpp"
# include <ctime>

class ChannelBot
{
private:
	Channel								*_chan;	//ChannelBot is contained within this channel, so this may be bad practice but I'm not sure how else to do this
	std::map<std::string, privilege>	_users;
	bool								_decay;
	std::time_t							_expiry;	//every second I will have my Channels check whether the bot should be destroyed, so I should maybe use multiple threads
public:
	ChannelBot(Channel *chan);
	~ChannelBot();
	int		checkUser(std::string hostmask);
	void	toggleDecay();
	bool	checkExpired();
	void	addUser(std::string hostmask, privilege priv);
	void	inviteUser(std::string hostmask);
	// I'll have the destructor manage this, in Channel the ChannelBot will be a ChannelBot pointer, and it will be allocated for on the heap, then deleted when no longer needed
};

#endif