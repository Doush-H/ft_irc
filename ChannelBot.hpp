#ifndef CHANNELBOT_HPP
# define CHANNELBOT_HPP

# include <pthread.h>
# include <unistd.h>
# include <map>
# include "User.hpp"
# include "enums.hpp"

class	Channel;

class ChannelBot
{
private:
	User								_botUser;
	bool								_botEnabled;
	bool								_despawnBot;
	bool								_isReal;
	std::map<std::string, privilege>	_userHistory;
	pthread_mutex_t						_enabledMutex;
	pthread_t							_botThread;
public:
	ChannelBot();
	ChannelBot(Channel &chan);
	ChannelBot(ChannelBot &bot);
	ChannelBot	&operator = (ChannelBot &bot);
	~ChannelBot();
	User		getBotUser() const;
	void		setBotEnabled(bool enabled);
	bool		getBotEnabled();
	void		setDespawnBot(bool enabled);
	bool		getDespawnBot();
	bool		getIsReal() const;
	void		setIsReal(bool real);
	privilege	checkUserHistory(User &user);
	std::map<std::string, privilege>	getUserHistory() const;
	void		beginThread(Channel &chan);
	// I'll have the destructor manage this, in Channel the ChannelBot will be a ChannelBot pointer, and it will be allocated for on the heap, then deleted when no longer needed
};

#endif