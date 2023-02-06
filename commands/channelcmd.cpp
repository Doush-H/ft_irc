#include "Server.hpp"

// -------------------------------- JOIN --------------------------------

std::map<User, std::string> Server::joinCommand(Message& msg){
	std::map<User, std::string> resp;
	std::list<std::string> params = msg.getParams();


	if (msg.getParams().size() < 1 || msg.getParams().size() > 2) {	//command valid for 1 to 2 params
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 :Not all parameters were provided");
	} else if (!msg.getSenderUser().isRegistered()) {	//check if user not registered yet
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 :Please log in before joining any channels");
	} else {
		std::string chanName = params.front();
		std::string successfulJoin = ":" + msg.getSenderUser().getNick() + "!" + msg.getSenderUser().getName() + "@127.0.0.1 JOIN :" + chanName;
		std::map<std::string, Channel>::iterator	chan = _channels.find(chanName);
		if (chan != _channels.end()) {	//case channel is found, join it as a regular user (no priority)
			std::string key = params.back();
			if (msg.getParams().size() == 2 && chan->second.getChannelKey() == key) {	//if key is required, accept if key is correct
				chan->second.addUser(msg.getSenderUser(), NO_PRIO);
				sendInfoToNewJoin(msg, &(chan->second), &resp);
				sendToChannel(&resp, _channels.find(chanName)->second, successfulJoin); // send the join message to the whole channel to inform everyone that a new user joined the channel (it's specified by the norm). In order to also inform the new user that he successfuly joined the channel call the function after adding the user to the channel
			} else if (msg.getParams().size() == 2) {	//else reject if key is incorrect
				addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "475 :Cannot join channel, invalid key");
			} else {	//if no key then join the channel directly
				chan->second.addUser(msg.getSenderUser(), NO_PRIO);
				sendInfoToNewJoin(msg, &(chan->second), &resp);
				sendToChannel(&resp, _channels.find(chanName)->second, successfulJoin);
			}
		} else {	//if channel does not exist, create one and add the user as an operator
			Channel	newchan	= Channel(chanName, NONE);
			newchan.addUser(msg.getSenderUser(), OPERATOR);
			_channels.insert(std::pair<std::string, Channel>(chanName, newchan));
			sendInfoToNewJoin(msg, &(newchan), &resp);
		}
	}
	return resp;
}

void Server::sendInfoToNewJoin(Message& msg, const Channel* channel, std::map<User, std::string>* resp) {
	// ------------------ send the join confirmation ---------------------
	std::string senderPrefix = ":" + msg.getSenderUser().getNick() + "!" + msg.getSenderUser().getName() + "@127.0.0.1";
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

	if (msg.getParams().size() != 1) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 :Not all parameters were provided");
	} else if (!msg.getSenderUser().isRegistered()) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 :Please log in before joining any channels");
	} else {
		std::map<std::string, Channel>::iterator	chan = _channels.find(msgParams.front());
		if (chan == _channels.end()){
			addResponse(&resp, msg.getSenderUser(), SERV_PREFIX + msg.getSenderUser().getNick() \
				+ " " + msgParams.front() + " 403 :No such channel");
		} else if (chan->second.findUser(msg.getSenderUser()) == -1){
			addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "442 :You're not on that channel");
		} else {
			sendToChannel(&resp, chan->second, ":" + msg.getSenderUser().getNick() + "!" \
				+ msg.getSenderUser().getName() + "@127.0.0.1 PART " + msgParams.front()); // inform everyone in the channel (including the user that's leaving) that the user is leaving the channel
			chan->second.removeUser(msg.getSenderUser());
			// addResponse(&resp, msg.getSenderUser(),":" + msg.getSenderUser().getNick() + "!" \
			// 	+ msg.getSenderUser().getName() + "@127.0.0.1 PART " + msgParams.front()); LEAVING THIS BEACUSE DIDN'T TEST THE CHANGE YET
		}
	}
	return resp;
}

// -------------------------------- TOPIC --------------------------------

std::map<User, std::string>	Server::topicCommand(Message& msg)
{
	std::map<User, std::string> resp;
	std::list<std::string> msgParams = msg.getParams();

	if (msg.getParams().size() < 1 || msg.getParams().size() > 2) {	//command valid for 1 to 2 params
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 :Not all parameters were provided");
	} else if (!msg.getSenderUser().isRegistered()) {	//check if user not registered yet
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 :Please log in before using this command");
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
		if (!chan->second.checkModes(TOPIC_RESTRICTED) || chan->second.findUser(msg.getSenderUser()) == OPERATOR) {	//if user has the right privileges then set new topic
			chan->second.setTopic(msgParams.back());
			sendToChannel(&resp, chan->second, ":" + msg.getSenderUser().getNick() + "!" \
				+ msg.getSenderUser().getName() + "@127.0.0.1 TOPIC " + msgParams.front() + " :" + msgParams.back()); // Topic change should also inform everyone in the channel
		} else {	//otherwise reject change and return relevant error
			addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "482 " + msg.getSenderUser().getNick() + " " \
				+ msgParams.front() + " :You're not channel operator");
		}
	}
	return resp;
}
