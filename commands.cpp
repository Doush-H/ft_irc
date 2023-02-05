#include "Server.hpp"

void Server::addResponse(std::map<User, std::string>* resp, const User& receiver, const std::string& respMessage) {
	resp->insert(std::pair<User, std::string>(receiver, respMessage));
}


void Server::checkIfRegistered(Message& msg, std::map<User, std::string>* resp) {
	if (!msg.getSenderUser().getFullName().empty() && !msg.getSenderUser().getNick().empty() && msg.getSenderUser().isAllowConnection() && !msg.getSenderUser().isRegistered()) {
		addResponse(resp, msg.getSenderUser(), SERV_PREFIX "001 " + msg.getSenderUser().getNick() + " :Welcome to our irc server " + msg.getSenderUser().getNick());
		msg.getSenderUser().setIsRegistered(true);
	}
}

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

std::map<User, std::string> Server::capCommand(Message& msg){
	std::map<User, std::string> resp;
	if (msg.getParams().front() == "LS")
		addResponse(&resp, msg.getSenderUser(), "CAP * LS :End of CAP LS negotiation");
	return resp;
}

std::map<User, std::string> Server::joinCommand(Message& msg){
	std::map<User, std::string> resp;
	std::list<std::string> params = msg.getParams();

	if (msg.getParams().size() < 1 || msg.getParams().size() > 2) {	//command valid for 1 to 2 params
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 :Not all parameters were provided");
	} else if (!msg.getSenderUser().isRegistered()) {	//check if user not registered yet
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 :Please log in before joining any channels");
	} else {
		std::string chanName = params.front();
		std::map<std::string, Channel>::iterator	chan = _channels.find(chanName);
		if (chan != _channels.end()) {	//case channel is found, join it as a regular user (no priority)
			std::string key = params.back();
			if (msg.getParams().size() == 2 && chan->second.getChannelKey() == key) {	//if key is required, accept if key is correct
				addResponse(&resp, msg.getSenderUser(), ":" + msg.getSenderUser().getNick() + "!" \
					+ msg.getSenderUser().getName() + "@127.0.0.1 JOIN :" + chanName);
				chan->second.addUser(msg.getSenderUser(), NO_PRIO);
			} else if (msg.getParams().size() == 2) {	//else reject if key is incorrect
				addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "475 :Cannot join channel, invalid key");
			} else {	//if no key then join the channel directly
				chan->second.addUser(msg.getSenderUser(), NO_PRIO);
				addResponse(&resp, msg.getSenderUser(), ":" + msg.getSenderUser().getNick() + "!" \
					+ msg.getSenderUser().getName() + "@127.0.0.1 JOIN :" + chanName);
			}
		} else {	//if channel does not exist, create one and add the user as an operator
			Channel	newchan	= Channel(chanName, 0);
			newchan.addUser(msg.getSenderUser(), OPERATOR);
			_channels.insert(std::pair<std::string, Channel>(chanName, newchan));
			addResponse(&resp, msg.getSenderUser(), ":" + msg.getSenderUser().getNick() + "!" \
					+ msg.getSenderUser().getName() + "@127.0.0.1 JOIN :" + chanName);
		}
	}
	return resp;
}

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
			addResponse(&resp, msg.getSenderUser(), ":" + msg.getSenderUser().getNick() + "!" \
				+ msg.getSenderUser().getName() + "@127.0.0.1 TOPIC " + msgParams.front() + " :" + msgParams.back());
		} else {	//otherwise reject change and return relevant error
			addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "482 " + msg.getSenderUser().getNick() + " " \
				+ msgParams.front() + " :You're not channel operator");
		}
	}
	return resp;
}

std::map<User, std::string>	Server::whoCommand(Message& msg)
{
	//for now I will not do the operator flag, because it makes for a lot of if else statements
	std::map<User, std::string> resp;
	std::list<std::string> msgParams = msg.getParams();

	if (msg.getParams().size() > 1) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 :Not all parameters were provided");
	} else if (!msg.getSenderUser().isRegistered()) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 :Please log in before joining any channels");
	} else if (msg.getParams().size() == 0){
		std::map<int, User>::iterator	it = _users.begin();

		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "352 " + msg.getSenderUser().getNick() + " * " \
			+ msg.getSenderUser().getNick() + " 42irc.com " + msg.getSenderUser().getNick() + " :" \
			+ msg.getSenderUser().getFullName());
		for (; it != _users.end(); it++)
			addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "352 " + msg.getSenderUser().getNick() + " * " \
				+ msg.getSenderUser().getNick() + " 42irc.com " + it->second.getNick() + " :" \
				+ it->second.getFullName());
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "315 " + msg.getSenderUser().getNick() + " * " \
			+ msg.getSenderUser().getNick() + " 42irc.com " + msg.getSenderUser().getNick() + " :" \
			+ msg.getSenderUser().getFullName());
	} else if (msg.getParams().size() == 1){
		std::map<std::string, Channel>::iterator	chan = _channels.find(msgParams.front());
		if (chan == _channels.end())
			addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "403 " + msg.getSenderUser().getNick() \
				+ " " + msgParams.front() + " :No such channel");
		else
		{
			addResponse(&resp, msg.getSenderUser(), chan->second.constructWho(msg.getSenderUser()));
			// problem: I need to now be able to iterate through the _users list inside of _channels, but i can't
		}
	}
	return resp;
}

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

std::list<std::string> getUsers(std::string str);

void Server::privmsgToUserCommand(Message& msg, std::map<User, std::string>* resp) {
	std::string senderPrefix = ":" + msg.getSenderUser().getNick() + "!" + msg.getSenderUser().getName();
	std::string senderMessage = msg.getParams().back();
	std::list<std::string> users = getUsers(msg.getParams().front());
	std::list<std::string>::iterator it = users.begin();
	while (it != users.end()) {
		std::map<int, User>::iterator userIt = findUser(*it);
		if (userIt != _users.end()) {
			addResponse(resp, userIt->second, senderPrefix + " PRIVMSG " + userIt->second.getNick() + " :" + senderMessage);
		} else {
			resp->clear();
			addResponse(resp, msg.getSenderUser(), SERV_PREFIX "401 " + msg.getSenderUser().getNick() + " " + *it + " :No such nick/channel");
		}
		it++;
	}
}

std::map<int, User>::iterator Server::findUser(std::string nickName) {
	std::map<int, User>::iterator it = _users.begin();
	while (it != _users.end()) {
		if (it->second.getNick() == nickName) {
			return it;
		}
		it++;
	}
	return it;
}

std::list<std::string> getUsers(std::string str) {
	char* buf = const_cast<char*>(str.c_str());
	char *user = strtok(buf, ",");

	std::list<std::string> userList;
	while (user != NULL) {
		userList.push_back(user);
		user = strtok(NULL, ",");
	}
	return userList;
}

void Server::privmsgToChannelCommand(Message& msg, std::map<User, std::string>* resp) {

}