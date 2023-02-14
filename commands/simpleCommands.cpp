#include "Server.hpp"

// -------------------------------- PING --------------------------------

std::map<User, std::string> Server::pingCommand(Message& msg){
	std::map<User, std::string> resp;
	if (msg.getParams().size() < 1 || msg.getParams().size() > 2) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 " + msg.getSenderUser().getNick() + " " + msg.getCommand() + " :Wrong number of parameters");
	} else {
		std::list<std::string> params = msg.getParams();
		addResponse(&resp, msg.getSenderUser(), "PONG :" + params.front());
	}
	return resp;
}

// -------------------------------- CAP --------------------------------

std::map<User, std::string> Server::capCommand(Message& msg){
	std::map<User, std::string> resp;
	if (msg.getParams().front() == "LS")
		addResponse(&resp, msg.getSenderUser(), "CAP * LS :End of CAP LS negotiation");
	return resp;
}

// -------------------------------- QUIT --------------------------------

std::map<User, std::string> Server::quitCommand(Message& msg) {
	std::map<User, std::string> resp;

	std::string userPrefix = ":" + msg.getSenderUser().getNick() + "!" + msg.getSenderUser().getName();
	if (msg.getParams().size() > 1) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 " + msg.getSenderUser().getNick() + " " + msg.getCommand() + " :Too many params");
	} else {
		msg.getSenderUser().setDisconnect(true); // need to set it here so it effects the user in this iteration, the returned map has a copy of user not pointer
		std::map<std::string, Channel>::iterator chanIt = _channels.begin();
		std::string quitMessage = userPrefix + " QUIT";
		if (msg.getParams().size() == 1)
			quitMessage += " :" + msg.getParams().back();
		while (chanIt != _channels.end()) {
			if (chanIt->second.findUser(msg.getSenderUser()) != -1) {
				sendToChannel(&resp, chanIt->second, quitMessage);
				chanIt->second.removeUser(msg.getSenderUser());
				if (chanIt->second.countUsers() == 0)
					_channels.erase(chanIt->first);
			}
			chanIt++;
			if (chanIt->second.countUsers() == 0)
				_channels.erase(chanIt);
		}
		addResponse(&resp, msg.getSenderUser(), quitMessage);
	}

	return resp;
}

// -------------------------------- LIST --------------------------------

std::map<User, std::string> Server::listCommand(Message& msg) {
	std::map<User, std::string> resp;
	std::list<std::string> paramsList = msg.getParams();
	if (paramsList.size() > 1) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 " + msg.getSenderUser().getNick() + " " + msg.getCommand() + " :Too many params");
	} else if (!msg.getSenderUser().isRegistered()) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 " + msg.getSenderUser().getNick() + " :Please log in before using this command");
	} else if (paramsList.size() == 0) {
		listChannels(&resp, &msg, "");
	} else {
		listChannels(&resp, &msg, paramsList.front());
	}

	return resp;
}

void Server::listChannels(std::map<User, std::string>* resp, Message* msg, std::string channelsStr) {

	addResponse(resp, msg->getSenderUser(), SERV_PREFIX "321 " + msg->getSenderUser().getNick() + " Channel :Users Name");
	if (!channelsStr.empty()) {
		std::list<std::string> channelsList = getRecieversFromInputList(msg->getParams().front());
		std::list<std::string>::const_iterator it = channelsList.begin();
		while (it != channelsList.end()) {
			std::map<std::string, Channel>::const_iterator chanIt = _channels.find(*it);
			if (chanIt == _channels.end()) {
				resp->clear();
				addResponse(resp, msg->getSenderUser(), SERV_PREFIX "403 " + msg->getSenderUser().getNick() + " " + *it + " :No such channel");
				return ;
			}
			std::string chanTopic = ":" + chanIt->second.getTopic();
			std::stringstream ss;
			ss << chanIt->second.countUsers();
			std::string chanUserCount = ss.str();

			std::string chanInfoMsg = SERV_PREFIX "322 " + msg->getSenderUser().getNick() + " " + *it + " " + chanUserCount + " " + chanTopic;
			addResponse(resp, msg->getSenderUser(), chanInfoMsg);
			it++;
		}
	} else {
		std::map<std::string, Channel>::const_iterator chanIt = _channels.begin();
		while (chanIt != _channels.end()) {
			std::string chanTopic = ":" + chanIt->second.getTopic();
			std::stringstream ss;
			ss << chanIt->second.countUsers();
			std::string chanUserCount = ss.str();

			std::string chanInfoMsg = SERV_PREFIX "322 " + msg->getSenderUser().getNick() + " " + chanIt->first + " " + chanUserCount + " " + chanTopic;
			addResponse(resp, msg->getSenderUser(), chanInfoMsg);
			chanIt++;
		}
	}
	addResponse(resp, msg->getSenderUser(), SERV_PREFIX "323 " + msg->getSenderUser().getNick() + " :End of /LIST");
}

// -------------------------------- INVITE --------------------------------

std::map<User, std::string> Server::inviteCommand(Message& msg) {
	std::map<User, std::string> resp;

	std::string senderPrefix = ":" +  msg.getSenderUser().getNick() + "!" + msg.getSenderUser().getName() + " ";
	std::string chanName = msg.getParams().back();
	std::string userNick = msg.getParams().front();

	if (!msg.getSenderUser().isRegistered()) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 " + msg.getSenderUser().getNick() + " :Please log in before using this command");
		return resp;
	}

	// Checking the param size
	if (msg.getParams().size() != 2) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 " + msg.getSenderUser().getNick() + " " + msg.getCommand() + " :Wrong number of params");
		return resp;
	}

	std::map<int, User>::iterator userIt = findUserByNick(userNick);

	// Checking if the user exists
	if (userIt == _users.end()) { 
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "401 " + msg.getSenderUser().getNick() + " " + userNick + " :No such nick");
		return resp;
	}


	std::map<std::string, Channel>::iterator chanIt = _channels.find(chanName);
	// Checking if the channel exists, if not then just invite the user and notify the sender that the user was invited
	if (chanIt == _channels.end()) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "341 " + msg.getSenderUser().getNick() + " " + chanName);
		addResponse(&resp, findUserByNick(userNick)->second, senderPrefix + "INVITE " + userNick + " " + chanName);
		return resp;
	}

	Channel* channel = &chanIt->second;
	int senderPriv = channel->findUser(msg.getSenderUser());
	//Check if sender is in the channel
	if (senderPriv == -1) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "442 " + msg.getSenderUser().getNick() + " " + chanName + " :You're not on that channel");
		return resp;
	}

	// Check if channel is invite only and sender user is not an operator he can't invite people
	if (channel->checkModes(INVITE_ONLY) && senderPriv != OPERATOR) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "482 " + msg.getSenderUser().getNick() + " " + chanName + " :You're not a channel operator");
		return resp;
	}

	// Check if the invited user is already in the channel
	int invitedUserPriv = channel->findUser(userIt->second);
	if (invitedUserPriv != -1) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "482 " + msg.getSenderUser().getNick() + " " + userNick + " " + chanName + " :User already in the channel");
		return resp;
	}

	// Everything's fine invite the user
	sendToChannel(&resp, *channel, SERV_PREFIX "341 " + msg.getSenderUser().getNick() + " " + userNick + " " + chanName);
	addResponse(&resp, findUserByNick(userNick)->second, senderPrefix + "INVITE " + userNick + " " + chanName);
	channel->addUser(userIt->second, INVITED);
	// channel->setPrivilege(findUserByNick(userNick)->second, INVITED);

	return resp;
}

