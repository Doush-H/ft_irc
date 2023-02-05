#include "Server.hpp"

// ------------------------------- Helper Functions ----------------------------------

void Server::addResponse(std::map<User, std::string>* resp, const User& receiver, const std::string& respMessage) {
	resp->insert(std::pair<User, std::string>(receiver, respMessage));
}

void Server::sendToChannel(std::map<User, std::string>* resp, Channel* channel, const std::string& message) {
	std::map<const User *, privilege> users = channel->getUsersMap();
	std::map<const User *, privilege>::const_iterator it = users.begin();

	while (it != users.end()) {
		addResponse(resp, *(it->first), message);
		it++;
	}

}

bool containsMask(const char* mask, const char* str) {
	if (mask[0] == '*') {
		do {
			if (containsMask(mask + 1, str)) {
				return true;
			}
		} while (*(str++));
		return false;
	} else if (mask[0] != str[0]) {
		return false;
	} else if (*str) {
		return containsMask(mask + 1, str + 1);
	}
	return true;
}

void Server::checkIfRegistered(Message& msg, std::map<User, std::string>* resp) {
	if (!msg.getSenderUser().getFullName().empty() && !msg.getSenderUser().getNick().empty() && msg.getSenderUser().isAllowConnection() && !msg.getSenderUser().isRegistered()) {
		addResponse(resp, msg.getSenderUser(), SERV_PREFIX "001 " + msg.getSenderUser().getNick() + " :Welcome to our irc server " + msg.getSenderUser().getNick());
		msg.getSenderUser().setIsRegistered(true);
	}
}

std::map<int, User>::iterator Server::findUserByNick(std::string nickName) {
	std::map<int, User>::iterator it = _users.begin();
	while (it != _users.end()) {
		if (it->second.getNick() == nickName) {
			return it;
		}
		it++;
	}
	return it;
}

std::list<std::string> getUsersFromInputList(std::string str) {
	char* buf = const_cast<char*>(str.c_str());
	char *user = strtok(buf, ",");

	std::list<std::string> userList;
	while (user != NULL) {
		userList.push_back(user);
		user = strtok(NULL, ",");
	}
	return userList;
}

// ------------------------------- Command functions ----------------------------------

// -------------------------------- PASS --------------------------------

std::map<User, std::string> Server::passCommand(Message& msg){
	std::map<User, std::string> resp;
	if (msg.getParams().size() != 1) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 :Wrong number of parameters");
	} else if (msg.getSenderUser().isRegistered()) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 :You're already registered");
	} else {
		std::list<std::string> params = msg.getParams();
		std::string inputPass = params.front();
		if (inputPass == _password) {
			msg.getSenderUser().setAllowConnection(true);
		} else {
			msg.getSenderUser().setAllowConnection(false);
			addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "464 * :Password incorrect");
		}
	}
	return resp;
}

// -------------------------------- NICK --------------------------------

std::map<User, std::string> Server::nickCommand(Message& msg){
	std::map<User, std::string> resp;
	if (msg.getParams().size() != 1) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "431 :No nick name was given");
	} else if (!msg.getSenderUser().isAllowConnection()) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 :Please provide the server password with PASS command before registration");
	} else {
		std::list<std::string> params = msg.getParams();
		std::string inputNick = params.front();
		std::map<int, User>::iterator it = _users.begin();
		std::map<int, User>::iterator eit = _users.end();

		bool setName = true;
		while (it != eit) {
			if (it->second.getNick() == inputNick) {
				setName = false;
				addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "433 " + inputNick + " :Nickname is already in use");
			}
			it++;
		}
		if (setName)
			msg.getSenderUser().setNick(inputNick);
	}
	checkIfRegistered(msg, &resp);
	return resp;
}

// -------------------------------- USER --------------------------------

std::map<User, std::string> Server::userCommand(Message& msg){
	std::map<User, std::string> resp;
	if (msg.getParams().size() != 4) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 :Not all parameters were provided");
	} else if (!msg.getSenderUser().isAllowConnection()) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 :Please provide the server password with PASS command before registration");
	} else if (msg.getSenderUser().isRegistered()) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 :You can not change your user details after registration");
	} else {
		std::list<std::string> params = msg.getParams();
		std::string inputName = params.front();
		std::string inputFullName = params.back();
		msg.getSenderUser().setName(inputName);
		msg.getSenderUser().setFullName(inputFullName);
	}
	checkIfRegistered(msg, &resp);
	return resp;
}

// -------------------------------- PING --------------------------------

std::map<User, std::string> Server::pingCommand(Message& msg){
	std::map<User, std::string> resp;
	if (msg.getParams().size() < 1 || msg.getParams().size() > 2) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 :Wrong number of parameters");
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
				sendToChannel(&resp, &(_channels.find(chanName)->second), successfulJoin); // send the join message to the whole channel to inform everyone that a new user joined the channel (it's specified by the norm). In order to also inform the new user that he successfuly joined the channel call the function after adding the user to the channel
			} else if (msg.getParams().size() == 2) {	//else reject if key is incorrect
				addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "475 :Cannot join channel, invalid key");
			} else {	//if no key then join the channel directly
				chan->second.addUser(msg.getSenderUser(), NO_PRIO);
				sendToChannel(&resp, &(_channels.find(chanName)->second), successfulJoin);
			}
		} else {	//if channel does not exist, create one and add the user as an operator
			Channel	newchan	= Channel(chanName, 0);
			newchan.addUser(msg.getSenderUser(), OPERATOR);
			_channels.insert(std::pair<std::string, Channel>(chanName, newchan));
			sendToChannel(&resp, &(_channels.find(chanName)->second), successfulJoin);
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
			// addResponse(&resp, msg.getSenderUser(), ":" + msg.getSenderUser().getNick() + "!" \
			// 	+ msg.getSenderUser().getName() + "@127.0.0.1 TOPIC " + msgParams.front() + " :" + msgParams.back()); // not removing this because didn't test it yet
			sendToChannel(&resp, &chan->second, ":" + msg.getSenderUser().getNick() + "!" \
				+ msg.getSenderUser().getName() + "@127.0.0.1 TOPIC " + msgParams.front() + " :" + msgParams.back()); // Topic change should also inform everyone in the channel
		} else {	//otherwise reject change and return relevant error
			addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "482 " + msg.getSenderUser().getNick() + " " \
				+ msgParams.front() + " :You're not channel operator");
		}
	}
	return resp;
}

// -------------------------------- WHO --------------------------------

std::map<User, std::string>	Server::whoCommand(Message& msg)
{
	std::map<User, std::string> resp;
	std::list<std::string> msgParams = msg.getParams();

	if (msg.getParams().size() > 2) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 :Not all parameters were provided");
	} else if (!msg.getSenderUser().isRegistered()) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 :Please register");
	} else if (msg.getParams().size() == 0){
		whoEveryone(&resp, &msg, "");
	} else if (msg.getParams().size() == 1){
		whoOneParam(&resp, &msg);
	} else if (msg.getParams().size() == 2) {
		whoTwoParam(&resp, &msg);
	}
	return resp;
}

void Server::whoEveryone(std::map<User, std::string>* resp, Message* msg, const std::string& mask) {
	std::string respMessage = "";
	std::map<int, User>::const_iterator it = _users.begin();
	while (it != _users.end()) {
		if ((mask.empty() || containsMask(mask.c_str(), msg->getSenderUser().getNick().c_str())) && it->second.getNick() != msg->getSenderUser().getNick())
			respMessage.append(SERV_PREFIX "352 " + msg->getSenderUser().getNick() + " * " + it->second.getName() + " * 42irc.com " + it->second.getNick() + " H :0 " + it->second.getFullName() + "\r\n");
		it++;
	}
	respMessage += SERV_PREFIX "315 " + msg->getSenderUser().getNick() + "* :End of /WHO list";
	addResponse(resp, msg->getSenderUser(), respMessage);

}

void Server::whoChannel(std::map<User, std::string>* resp, Message* msg, const Channel& channel, int priv) {
	std::string respMessage = "";
	std::map<const User *, privilege>::const_iterator it = channel.getUsersMap().begin();
	if (channel.findUser(msg->getSenderUser()) != -1) { // check if the user belongs to the channel
		while (it != channel.getUsersMap().end()) {
			if ((priv == -1 || it->second == priv) && it->first->getNick() != msg->getSenderUser().getNick()) {
				respMessage.append(SERV_PREFIX "352 " + msg->getSenderUser().getNick() + " " + channel.getName() + " " + it->first->getName() + " * 42irc.com " + it->first->getNick() + " :0 " + it->first->getFullName() + "\r\n");
			}
			it++;
		}
	}
	respMessage += SERV_PREFIX "315 " + msg->getSenderUser().getNick() + " " + channel.getName() +  " :End of /WHO list";
	addResponse(resp, msg->getSenderUser(), respMessage);

}

void Server::whoOneParam(std::map<User, std::string>* resp, Message* msg) {
	std::string maskParam = msg->getParams().front();

	if (maskParam.at(0) == '#' || maskParam.at(0) == '&') {
		std::map<std::string, Channel>::const_iterator it = _channels.find(maskParam);
		if (it == _channels.end()) { // if the channel was not found return error
			addResponse(resp, msg->getSenderUser(), SERV_PREFIX "403 " + msg->getSenderUser().getNick() \
				+ " " + maskParam + " :No such channel");
		} else { // else send the list of people in the channel with all privileges
			whoChannel(resp, msg, it->second, -1);
		}
	} else {
		whoEveryone(resp, msg, maskParam);
	}
}

void Server::whoTwoParam(std::map<User, std::string>* resp, Message* msg) {
	std::string maskParam = msg->getParams().front();
	std::string secondParam = msg->getParams().back();

	if (secondParam != "o") {
		addResponse(resp, msg->getSenderUser(), SERV_PREFIX "461 " + msg->getCommand() + " :Not all parameters were provided");
	} else if (maskParam.at(0) == '#' || maskParam.at(0) == '&') {
		std::map<std::string, Channel>::const_iterator it = _channels.find(maskParam);
		if (it == _channels.end()) { // if the channel was not found return error
			addResponse(resp, msg->getSenderUser(), SERV_PREFIX "403 " + msg->getSenderUser().getNick() \
				+ " " + maskParam + " :No such channel");
		} else { // else send a list of people in the channel with OPERATOR privileges
			whoChannel(resp, msg, it->second, OPERATOR);
		}
	} else { // if not channel and the option 'o' was specified return end of list because we don't have server operators
		addResponse(resp, msg->getSenderUser(), SERV_PREFIX "315 " + msg->getSenderUser().getNick() + "* :End of /WHO list");
	}
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
			sendToChannel(&resp, &chan->second, ":" + msg.getSenderUser().getNick() + "!" \
				+ msg.getSenderUser().getName() + "@127.0.0.1 PART " + msgParams.front()); // inform everyone in the channel (including the user that's leaving) that the user is leaving the channel
			chan->second.removeUser(msg.getSenderUser());
			// addResponse(&resp, msg.getSenderUser(),":" + msg.getSenderUser().getNick() + "!" \
			// 	+ msg.getSenderUser().getName() + "@127.0.0.1 PART " + msgParams.front()); LEAVING THIS BEACUSE DIDN'T TEST THE CHANGE YET
		}
	}
	return resp;
}

// -------------------------------- PRIVMSG --------------------------------


std::map<User, std::string> Server::privmsgCommand(Message& msg) {
	std::map<User, std::string> resp;
	std::list<std::string> msgParams = msg.getParams();

	if (msgParams.size() != 2) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 :Not all parameters were provided");
	} else if (msgParams.front().at(0) == '#' || msgParams.front().at(0) == '&') {
		privmsgToChannelCommand(msg, &resp);
	} else {
		privmsgToUserCommand(msg, &resp);
	}
	return resp;
}

void Server::privmsgToUserCommand(Message& msg, std::map<User, std::string>* resp) {
	std::string senderPrefix = ":" + msg.getSenderUser().getNick() + "!" + msg.getSenderUser().getName();
	std::string senderMessage = msg.getParams().back();
	std::list<std::string> users = getUsersFromInputList(msg.getParams().front());
	std::list<std::string>::iterator it = users.begin();
	while (it != users.end()) {
		std::map<int, User>::iterator userIt = findUserByNick(*it);
		if (userIt != _users.end()) {
			addResponse(resp, userIt->second, senderPrefix + " PRIVMSG " + userIt->second.getNick() + " :" + senderMessage);
		} else {
			resp->clear();
			addResponse(resp, msg.getSenderUser(), SERV_PREFIX "401 " + msg.getSenderUser().getNick() + " " + *it + " :No such nick/channel");
		}
		it++;
	}
}

void Server::privmsgToChannelCommand(Message& msg, std::map<User, std::string>* resp) {
 // Didn't have time to work on this again :D
}