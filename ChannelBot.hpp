#ifndef CHANNELBOT_HPP
# define CHANNELBOT_HPP

# include <pthread.h>
# include <unistd.h>
# include <map>

class	Channel;
enum	privilege;

class ChannelBot
{
private:
	User								*_botUser;
	bool								_botEnabled;
	bool								_despawnBot;
	std::map<std::string, privilege>	_userHistory;
	pthread_mutex_t						_enabledMutex;
	pthread_t							_botThread;
public:
	ChannelBot(Channel &chan);
	~ChannelBot();
	User		*getBotUser();
	void		setBotEnabled(bool enabled);
	bool		getBotEnabled();
	void		setDespawnBot(bool enabled);
	bool		getDespawnBot();
	privilege	checkUserHistory(User &user);
	// I'll have the destructor manage this, in Channel the ChannelBot will be a ChannelBot pointer, and it will be allocated for on the heap, then deleted when no longer needed
};

#endif