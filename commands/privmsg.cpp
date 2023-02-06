#include "Server.hpp"

std::list<std::string>	getRecieversFromInputList(std::string str);

// -------------------------------- PRIVMSG --------------------------------

std::map<User, std::string> Server::privmsgCommand(Message& msg) {
	std::map<User, std::string> resp;
	std::list<std::string> msgParams = msg.getParams();

	std::list<std::string> recievers = getRecieversFromInputList(msg.getParams().front());
	std::list<std::string>::iterator it = recievers.begin();
	while (it != recievers.end()) {
		if (msgParams.size() != 2) {
			addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 :Not all parameters were provided");
		} else if (it->at(0) == '#' || it->at(0) == '&') {
			if (!privmsgToChannelCommand(&msg, &resp, *it))
				break;
		} else {
			if (!privmsgToUserCommand(&msg, &resp, *it))
				break;
		}
		it++;
	}
	return resp;
}

bool Server::privmsgToUserCommand(Message* msg, std::map<User, std::string>* resp, const std::string& userNick) {
	std::string senderPrefix = ":" + msg->getSenderUser().getNick() + "!" + msg->getSenderUser().getName();
	std::string senderMessage = msg->getParams().back();
	std::map<int, User>::iterator userIt = findUserByNick(userNick);
	if (userIt != _users.end()) {
		addResponse(resp, userIt->second, senderPrefix + " PRIVMSG " + userIt->second.getNick() + " :" + senderMessage);
	} else {
		resp->clear();
		addResponse(resp, msg->getSenderUser(), SERV_PREFIX "401 " + msg->getSenderUser().getNick() + " " + userNick + " :No such nick/channel");
		return false;
	}
	return true;
}

bool Server::privmsgToChannelCommand(Message* msg, std::map<User, std::string>* resp, const std::string& chanName) {
	std::string senderPrefix = ":" + msg->getSenderUser().getNick() + "!" + msg->getSenderUser().getName();
	std::string senderMessage = msg->getParams().back();
	std::string cannotSendMessage = SERV_PREFIX "404 " + chanName + " :Cannot send to channel";

	std::list<std::string> msgParams = msg->getParams();

	std::map<std::string, Channel>::const_iterator chanIt = _channels.find(chanName);

	if (chanIt == _channels.end()) {
		resp->clear();
		addResponse(resp, msg->getSenderUser(), SERV_PREFIX "401 " + chanName  + " :No such nick/channel");
		return false;
	} else {
		const Channel& channel = chanIt->second;
		if (channel.findUser(msg->getSenderUser()) == -1) { // check if user is not in the channel, if that's the case send back error
			resp->clear();
			addResponse(resp, msg->getSenderUser(), cannotSendMessage);
			return false;
		} else { // if user in the channel then continue
			if (channel.checkModes(MODERATED) && channel.findUser(msg->getSenderUser()) != VOICE_PRIO && channel.findUser(msg->getSenderUser()) != OPERATOR) {
				resp->clear();
				addResponse(resp, msg->getSenderUser(), cannotSendMessage);
				return false;
			} else {
				sendMsgToChannel(resp, channel, msg);
			}
		}
	}
	return true;
}
