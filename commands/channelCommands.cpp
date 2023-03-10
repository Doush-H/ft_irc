#include "Server.hpp"
#include "ChannelBot.hpp"

std::list<std::string> getRecieversFromInputList(std::string str);

// -------------------------------- JOIN --------------------------------

static std::string	flagGlossary(privilege priv)
{
	std::cout << priv << std::endl;
	switch (priv)
	{
	case OPERATOR:	//user modes
		return ("+o ");
	case VOICE_PRIO:
		return ("+v ");
	default:
		break;
	}
	return ("");
}

privilege	Server::checkPrivilege(Message& msg, Channel &chan, std::map<User, std::string>* resp)
{
	privilege	prio = NO_PRIO;
	if (chan.channelBot.getIsActive()) {
		std::string userHostmask = ":" + msg.getSenderUser().getNick() + "!" + msg.getSenderUser().getName() \
			+ "@" + msg.getSenderUser().getHostmask();
		prio = chan.channelBot.checkUserHistory(msg.getSenderUser());
		std::string	s = flagGlossary(prio);
		sendToChannel(resp, chan, userHostmask + " MODE " + chan.getName() + " " + s + msg.getSenderUser().getNick());
	}
	return (prio);
}

std::map<User, std::string> Server::joinCommand(Message& msg){
	std::map<User, std::string> resp;
	std::list<std::string> params = msg.getParams();

	if (msg.getParams().size() < 1 || msg.getParams().size() > 2) {	//command valid for 1 to 2 params
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 " + msg.getSenderUser().getNick() + " " + msg.getCommand() + " :Not all parameters were provided");
	} else if (!msg.getSenderUser().isRegistered()) {	//check if user not registered yet
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 " + msg.getSenderUser().getNick() + " :Please log in before joining any channels");
	} else {
		std::list<std::string>	inputlist = getRecieversFromInputList(params.front());
		std::list<std::string>	paramlist = getRecieversFromInputList(params.back());	// get the password list
		std::list<std::string>::iterator	it = inputlist.begin();
		std::list<std::string>::iterator	itParams = paramlist.begin();

		if (inputlist.size() < paramlist.size()) {	// if too many passwords provided return error
			addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 " + msg.getSenderUser().getNick() + " " + msg.getCommand() + " :Too many parameters provided");
			return resp;
		}
		for (; it != inputlist.end(); it++) {
			std::string successfulJoin = ":" + msg.getSenderUser().getNick() + "!" + msg.getSenderUser().getName() \
				+ "@" + msg.getSenderUser().getHostmask() + " JOIN :" + *it;
			std::map<std::string, Channel>::iterator	chan = _channels.find(*it);

			if (chan == _channels.end()) {
				Channel	newchan	= Channel(*it, NONE);
				if (params.size() == 2 && itParams != paramlist.end()) {	// if password was provided set the password
					newchan.setChannelKey(*itParams);
					newchan.setModes(KEY_PROTECTED);
					itParams++;
				}
				newchan.addUser(msg.getSenderUser(), OPERATOR);
				_channels.insert(std::pair<std::string, Channel>(*it, newchan));
				sendInfoToNewJoin(msg, &(newchan), &resp);
			} else if (chan->second.checkModes(INVITE_ONLY)) {
				if (chan->second.findUser(msg.getSenderUser()) == INVITED) {
					sendToChannel(&resp, _channels.find(*it)->second, successfulJoin); // send the join message to the whole channel to inform everyone that a new user joined the channel
					chan->second.addUser(msg.getSenderUser(), NO_PRIO);
					sendInfoToNewJoin(msg, &(chan->second), &resp);
					//change to NO_PRIO
				} else {
					addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "473 " + msg.getSenderUser().getNick() + " " + chan->second.getName() + " :Cannot join channel, invite only (and you haven't been invited)");
					//reject message
				}
			} else {
				std::string key = params.back();
				if (params.size() == 2 && chan->second.checkModes(KEY_PROTECTED) && chan->second.getChannelKey() == key) {	//if key is required and the correct key was provided accept the person
					sendToChannel(&resp, _channels.find(*it)->second, successfulJoin); // send the join message to the whole channel to inform everyone that a new user joined the channel
					privilege	priv = checkPrivilege(msg, chan->second, &resp);
					chan->second.addUser(msg.getSenderUser(), priv);
					sendInfoToNewJoin(msg, &(chan->second), &resp);
					if (chan->second.channelBot.getIsActive()) {
						chan->second.channelBot.addUserHistory(msg.getSenderUser(), priv);
					}
				} else if (chan->second.checkModes(KEY_PROTECTED) && (params.size() == 1 || chan->second.getChannelKey() != key)) {	//else if key not provided or key not correct
					addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "475 " + msg.getSenderUser().getNick() + " " + chan->second.getName() + " :Cannot join channel, invalid key");
				} else {	//if no key then join the channel directly
					sendToChannel(&resp, _channels.find(*it)->second, successfulJoin);
					privilege	priv = checkPrivilege(msg, chan->second, &resp);
					chan->second.addUser(msg.getSenderUser(), priv);
					sendInfoToNewJoin(msg, &(chan->second), &resp);
					if (chan->second.channelBot.getIsActive()) {
						chan->second.channelBot.addUserHistory(msg.getSenderUser(), priv);
					}
				}
			}
		}
	}
	refreshList(&resp);
	return resp;
}

void Server::sendInfoToNewJoin(Message& msg, const Channel* channel, std::map<User, std::string>* resp) {
	// // ------------------ send the join confirmation ---------------------
	std::string senderPrefix = ":" + msg.getSenderUser().getNick() + "!" + msg.getSenderUser().getName() + "@" + msg.getSenderUser().getHostmask();
	std::string respString = senderPrefix + " JOIN :" + channel->getName() + "\r\n";

	// -------------------- send channel topic --------------------

	if (!channel->getTopic().empty())
		respString.append(SERV_PREFIX "332 " + msg.getSenderUser().getNick() + " " + channel->getName() + " :" + channel->getTopic() + "\r\n");

	// -------------------- send the user list ---------------------
	respString.append(SERV_PREFIX "353 " + msg.getSenderUser().getNick());
	if (channel->checkModes(MODERATED))
		respString.append(" + ");
	else
		respString.append(" = ");
	
	respString.append(channel->getName());
	respString.append(" :");
	const std::map<const User *, privilege>& users = channel->getUsersMap();
	std::map<const User *, privilege>::const_iterator it = users.begin();
	while (it != users.end()) {
		const User* user = it->first;
		privilege userPriv = it->second;
		if (userPriv == OPERATOR)
			respString.append("@");
		else if (userPriv == VOICE_PRIO)
			respString.append("+");
		respString.append(user->getNick());
		it++; // INCREMENT THE ITERATOR
		if (it != users.end()) // CHECK IF LAST ONE, DON'T add space if the last one
			respString.append(" ");
	}
	respString.append("\r\n" SERV_PREFIX "366 " + msg.getSenderUser().getNick() + " " + channel->getName() + " :End of /NAMES list");
	addResponse(resp, msg.getSenderUser(), respString);
}

// -------------------------------- PART --------------------------------

std::map<User, std::string>	Server::partCommand(Message& msg)
{
	std::map<User, std::string> resp;
	std::list<std::string> msgParams = msg.getParams();

	if (msgParams.size() < 1 || msgParams.size() > 2) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 " + msg.getSenderUser().getNick() + " " + msg.getCommand() + " :Not all parameters were provided");
	} else if (!msg.getSenderUser().isRegistered()) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 " + msg.getSenderUser().getNick() + " :Please log in before joining any channels");
	} else {
		std::list<std::string>	inputlist = getRecieversFromInputList(msgParams.front());
		std::list<std::string>::iterator	it = inputlist.begin();
		for (; it != inputlist.end(); it++)
		{
			std::map<std::string, Channel>::iterator	chan = _channels.find(*it);

			if (chan == _channels.end()){
				addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "403 " + msg.getSenderUser().getNick() + " " + *it + " :No such channel");
			} else if (chan->second.findUser(msg.getSenderUser()) == -1){
				addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "442 " + msg.getSenderUser().getNick() + " " + *it + " :You're not on that channel");
			} else {
				if (msgParams.size() == 1) {
					sendToChannel(&resp, chan->second, ":" + msg.getSenderUser().getNick() + "!" \
						+ msg.getSenderUser().getName() + "@" + msg.getSenderUser().getHostmask() + " PART " + *it);
						// inform everyone in the channel (including the user that's leaving) that the user is leaving the channel
					chan->second.removeUser(msg.getSenderUser());
				} else {
					sendToChannel(&resp, chan->second, ":" + msg.getSenderUser().getNick() + "!" \
						+ msg.getSenderUser().getName() + "@" + msg.getSenderUser().getHostmask() + " PART " + *it + " :" + msgParams.back());
						// inform everyone in the channel (including the user that's leaving) that the user is leaving the channel
					chan->second.removeUser(msg.getSenderUser());
				}
			}
			if (chan->second.countUsers() == 0)
				_channels.erase(chan);
		}
	}
	refreshList(&resp);
	return resp;
}

// -------------------------------- TOPIC --------------------------------

std::map<User, std::string>	Server::topicCommand(Message& msg)
{
	std::map<User, std::string> resp;
	std::list<std::string> msgParams = msg.getParams();

	if (msg.getParams().size() < 1 || msg.getParams().size() > 2) {	//command valid for 1 to 2 params
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 " + msg.getSenderUser().getNick() + " " + msg.getCommand() + " :Not all parameters were provided");
	} else if (!msg.getSenderUser().isRegistered()) {	//check if user not registered yet
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 " + msg.getSenderUser().getNick() + " :Please log in before using this command");
	} else if (msg.getParams().size() == 1){	//case check topic
		std::map<std::string, Channel>::iterator	chan = _channels.find(msgParams.front());
		if (_channels.find(msgParams.front()) != _channels.end()) {	//case channel found, print topic
			if (chan->second.getTopic().compare("No topic is set") == 0) {	//if there is no topic, return as such
				addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "331 " + msg.getSenderUser().getNick() \
					+ " " + msgParams.front() + " :No topic is set");
			} else {	//if there is a topic, return as such
				addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "332 " + msg.getSenderUser().getNick() \
					+ " " + msgParams.front() + " :" + chan->second.getTopic());
			}
		} else {	//if channel not found return relevant error
			addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "403 " + msg.getSenderUser().getNick() \
				+ " " + msgParams.front() + " :No such channel");
		}
	} else if (msg.getParams().size() == 2){	//case change topic
		std::map<std::string, Channel>::iterator	chan = _channels.find(msgParams.front());
		if (chan == _channels.end()) {
			addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "403 " + msg.getSenderUser().getNick() \
				+ " " + msgParams.front() + " :No such channel");
		} else if (!chan->second.checkModes(TOPIC_RESTRICTED) || chan->second.findUser(msg.getSenderUser()) == OPERATOR) {	//if user has the right privileges then set new topic
			chan->second.setTopic(msgParams.back());
			sendToChannel(&resp, chan->second, ":" + msg.getSenderUser().getNick() + "!" \
				+ msg.getSenderUser().getName() + "@" + msg.getSenderUser().getHostmask() + " TOPIC " + msgParams.front() + " :" + msgParams.back());
				// Topic change should also inform everyone in the channel
		} else {	//otherwise reject change and return relevant error
			addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "482 " + msg.getSenderUser().getNick() + " " \
				+ msgParams.front() + " :You're not channel operator");
		}
	}
	return resp;
}
 
// -------------------------------- KICK --------------------------------


void	Server::kickNoComment(std::map<User, std::string>* resp, Message msg, Channel &chan, User &rm)
{
	std::list<std::string> msgParams = msg.getParams();

	sendToChannel(resp, chan, ":" + msg.getSenderUser().getNick() + "!" \
		+ msg.getSenderUser().getName() + "@" + msg.getSenderUser().getHostmask() + " KICK " + msgParams.front() + " " + msgParams.back());
	chan.removeUser(rm);
}

void	Server::kickComment(std::map<User, std::string>* resp, Message msg, Channel &chan, User &rm)
{
	std::list<std::string> msgParams = msg.getParams();
	std::string	chanName = msgParams.front();
	msgParams.pop_front();

	sendToChannel(resp, chan, ":" + msg.getSenderUser().getNick() + "!" \
		+ msg.getSenderUser().getName() + "@" + msg.getSenderUser().getHostmask() + " KICK " + chanName + " " + msgParams.front() + " :" + msgParams.back());
	chan.removeUser(rm);
}

std::map<User, std::string>	Server::kickCommand(Message& msg)
{
	std::map<User, std::string> resp;
	std::list<std::string> msgParams = msg.getParams();
	std::list<std::string>::iterator modesIt = msgParams.begin();

	std::advance(modesIt, 1);
	if (msg.getParams().size() < 2 || msg.getParams().size() > 3) {	//command valid for 1 to 3 params
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 " + msg.getSenderUser().getNick() + " " + msg.getCommand() + " :Not all parameters were provided");
	} else if (!msg.getSenderUser().isRegistered()) {	//check if user not registered yet
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 " + msg.getSenderUser().getNick() + " :Please log in before using MODE on any channels");
	} else {
		std::map<std::string, Channel>::iterator	chan = _channels.find(msgParams.front());
		std::map<int, User>::iterator rm = findUserByNick(*modesIt);

		if (chan == _channels.end()) {
			addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "403 " + msg.getSenderUser().getNick() \
				+ " " + msgParams.front() + " :No such channel");
			return resp;
		}

		if (chan->second.findUser(msg.getSenderUser()) != OPERATOR) {
			addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "482 " + msg.getSenderUser().getNick() + " " \
				+ msgParams.front() + " :You're not channel operator");
			return resp;
		} 
		
		if (chan->second.findUser(rm->second) == -1 || chan->second.findUser(rm->second) == INVITED) {
			addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "441 " + msg.getSenderUser().getNick() + " " + msgParams.back() + " " + chan->first + " :They aren't on that channel");
			return resp;
		}
		
		if (msgParams.size() == 2) {
			kickNoComment(&resp, msg, chan->second, rm->second);
		} else if (msgParams.size() == 3) {
			kickComment(&resp, msg, chan->second, rm->second);
		}
		
		if (chan->second.countUsers() == 0) // if after kick channel is empty remove the channel
			_channels.erase(chan->first);
		
		refreshList(&resp);
	}
	return resp;
}