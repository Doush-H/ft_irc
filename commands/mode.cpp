#include "Server.hpp"

//flags:
//  channel:            |   user:
//  +i = invite only    |
//  +p = private        |   +v = voice privileges
//  +s = secret         |   +o = operator privileges
//  +t = topic restric. |
//  +m = moderated      |

//syntax in:
//  MODE #channel +m    -> puts a channel into moderated mode
//  MODE #channel +v-o user -> removes a user's operator privileges and gives him/her voice privileges

//syntax out:
//  :42irc.com 324 <sender nick> #channel +m    -> returns that channel has been put into moderated mode
//  to do: user modes
//	user modes can only be given by a channel operator to users in the channel
//	

static std::string	constructFlags(const Channel &chan)
{
	std::string	ret = "+";

	if (chan.checkModes(SECRET))
		ret += "s";
	if (chan.checkModes(PRIV))
		ret += "p";
	if (chan.checkModes(MODERATED))
		ret += "m";
	if (chan.checkModes(INVITE_ONLY))
		ret += "i";
	if (chan.checkModes(TOPIC_RESTRICTED))
		ret += "t";
	return ret;
}

// I need the actual character that is invalid, so I'm returning it as a negative to indicate bad input
static int	flagGlossary(char c)
{
	switch (c)
	{
	case 's':
		return (SECRET);
	case 'p':
		return (PRIV);
	case 'm':
		return (MODERATED);
	case 'i':
		return (INVITE_ONLY);
	case 't':
		return (TOPIC_RESTRICTED);
	case 'o':
		return (OPERATOR);
	case 'v':
		return (VOICE_PRIO);
	default:
		break;
	}
	return (c * -1);
}

// I will indicate flags to add in the least significant byte
// I will indicate flags to remove in the second least significant byte
// -1 will be an error case
static int	parseFlags(std::string str)
{
	int	flags = 0;
	bool	polarity = true;

	for (std::string::iterator it = str.begin(); it != str.end(); it++) {
		if (*it == '+') {
			polarity = true;
			continue ;
		} else if (*it == '-') {
			polarity = false;
			continue ;
		}
		int	temp = flagGlossary(*it);
		if (temp < 0)
			return (temp);
		if (polarity)
			flags |= temp;
		else
			flags |= temp << 8;
	}
	return flags;
}

void	Server::modeReturnFlags(std::map<User, std::string> *resp, Message &msg)
{
	std::list<std::string> msgParams = msg.getParams();
	//in this case I will return the current modes on the channel
	std::map<std::string, Channel>::iterator	chan = _channels.find(msgParams.front());
	if (chan == _channels.end()) {	//check if channel exists
		addResponse(resp, msg.getSenderUser(), SERV_PREFIX + msg.getSenderUser().getNick() \
			+ " " + msgParams.front() + " 403 :No such channel");
	} else {
		addResponse(resp, msg.getSenderUser(), SERV_PREFIX "324 " + msg.getSenderUser().getNick() \
			+ " " + msgParams.front() + " " + constructFlags(chan->second));
	}
}

void	Server::modeChangeChannel(std::map<User, std::string> *resp, Message &msg)
{
	std::list<std::string> msgParams = msg.getParams();
	std::string	chanName = msgParams.front();
	std::map<std::string, Channel>::iterator	chan = _channels.find(chanName);
	int	flags = parseFlags(msgParams.back());
	if (chan == _channels.end()) {	//check if channel exists
		addResponse(resp, msg.getSenderUser(), SERV_PREFIX + msg.getSenderUser().getNick() \
			+ " " + msgParams.front() + " 403 :No such channel");
	} else if (!msgParams.back().compare("b")) {	//to get rid of the annoying banmask request I just send back end of ban list
		addResponse(resp, msg.getSenderUser(), SERV_PREFIX "368 " + msg.getSenderUser().getNick() \
			+ " " + chanName + " :End of Channel Ban List");
	} else if (flags < 0) {	//check if flags are valid
		addResponse(resp, msg.getSenderUser(), SERV_PREFIX "472 " + msg.getSenderUser().getNick() \
			+ " " + static_cast <char> (std::abs(flags)) + " :is unknown mode char to me");
	} else {	//successful parsing
		int removeflags = flags >> 8;	//separate remove flags from add flags by bitshifting
		flags = flags << 24;
		flags = flags >> 24;
		chan->second.setModes(flags);
		chan->second.removeModes(removeflags);
		std::string	message = SERV_PREFIX "324 " + msg.getSenderUser().getNick() \
			+ " " + chanName + " " + msgParams.back();
		User	temp = msg.getSenderUser();
		addResponse(resp, msg.getSenderUser(), message);
		sendToChannel(resp, chan->second, ":" + temp.getNick() + "!" + temp.getName() \
			+ "@127.0.0.1 MODE " + msgParams.front() + " " + msgParams.back());
	}
}

void	Server::modeChangeChannelUser(std::map<User, std::string> *resp, Message &msg)
{
	std::list<std::string> msgParams = msg.getParams();
	std::string	chanName = msgParams.front();
	msgParams.pop_front();
	std::map<std::string, Channel>::iterator	chan = _channels.find(chanName);
	std::map<int, User>::iterator	user = findUserByNick(msgParams.back());
	int	flags = parseFlags(msgParams.front());
	if (chan == _channels.end()) {	//check if channel exists
		addResponse(resp, msg.getSenderUser(), SERV_PREFIX + msg.getSenderUser().getNick() \
			+ " " + chanName + " 403 :No such channel");
	} else if (flags < 0) {	//check if flags are valid
		addResponse(resp, msg.getSenderUser(), SERV_PREFIX "472 " + msg.getSenderUser().getNick() \
			+ " " + static_cast <char> (std::abs(flags)) + " :is unknown mode char to me");
	} else if (user == _users.end() || chan->second.findUser(user->second) == -1) {	//no such target user exists in channel
		addResponse(resp, msg.getSenderUser(), SERV_PREFIX + msg.getSenderUser().getNick() \
			+ " " + chanName + " 401 :No such nick/channel");
	} else if (chan->second.findUser(msg.getSenderUser()) != 1) {	//command user not operator
		addResponse(resp, msg.getSenderUser(), SERV_PREFIX + msg.getSenderUser().getNick() \
			+ " " + chanName + " 502 :Cant change mode for other users");
	} else {	//successful parsing
		int removeflags = flags >> 8;	//separate remove flags from add flags by bitshifting
		flags = flags << 24;
		flags = flags >> 24;
		if (flags & 1)
			chan->second.setPrivilege(user->second, OPERATOR);
		else if (flags & 2)
			chan->second.setPrivilege(user->second, VOICE_PRIO);
		if (removeflags & 2 || removeflags & 1)
			chan->second.setPrivilege(user->second, NO_PRIO);
		User	temp = msg.getSenderUser();
		std::string	message = SERV_PREFIX "324 " + temp.getNick() \
			+ " " + chanName + " " + msgParams.front() + " " + msgParams.back();
		addResponse(resp, msg.getSenderUser(), message);
		sendToChannel(resp, chan->second, ":" + temp.getNick() + "!" + temp.getName() \
			+ "@127.0.0.1 MODE " + chanName + " " + msgParams.front() + " " + msgParams.back());
	}
}

std::map<User, std::string>	Server::modeCommand(Message& msg)
{
	std::map<User, std::string> resp;
	std::list<std::string> msgParams = msg.getParams();

	if (msg.getParams().size() < 1 || msg.getParams().size() > 3) {	//command valid for 2 to 3 params
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 :Not all parameters were provided");
	} else if (!msg.getSenderUser().isRegistered()) {	//check if user not registered yet
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 :Please log in before joining any channels");
	} else if (msgParams.size() == 1) {
		modeReturnFlags(&resp, msg);
	} else if (msgParams.size() == 2) {
		modeChangeChannel(&resp, msg);
	} else if (msgParams.size() == 3) {
		modeChangeChannelUser(&resp, msg);
	}
	return resp;
}