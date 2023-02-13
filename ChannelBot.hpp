#ifndef CHANNELBOT_HPP
# define CHANNELBOT_HPP

// # include <pthread.h>
// # include <unistd.h>
# include <map>
# include <sys/time.h>
# include "User.hpp"
# include "enums.hpp"

class	Channel;

class ChannelBot
{
private:
	User								_botUser;
	bool								_isActive;
	std::map<std::string, privilege>	_userHistory;
	struct timeval						_timestamp;
public:
	ChannelBot();
	ChannelBot(Channel &chan);
	ChannelBot(ChannelBot &bot);
	ChannelBot	&operator = (ChannelBot &bot);
	~ChannelBot();
	User				getBotUser() const;
	void				setIsActive(bool active);
	bool				getIsActive() const;
	privilege			checkUserHistory(User &user);
	struct timeval& 	getTimestamp();

	std::map<std::string, privilege>	getUserHistory() const;
	// I'll have the destructor manage this, in Channel the ChannelBot will be a ChannelBot pointer, and it will be allocated for on the heap, then deleted when no longer needed
};

#endif