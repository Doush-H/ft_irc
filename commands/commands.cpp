#include "Server.hpp"

// ------------------------------- Helper Functions ----------------------------------

void Server::addResponse(std::map<User, std::string>* resp, const User& receiver, const std::string& respMessage) {
	
	std::pair<std::map<User, std::string>::iterator, bool> insertResult = resp->insert(std::pair<User, std::string>(receiver, respMessage));
	if (insertResult.second == false) {
		std::map<User, std::string>::iterator it = insertResult.first;
		it->second.append("\r\n" + respMessage);
	}
}

void Server::sendToChannel(std::map<User, std::string>* resp, const Channel& channel, const std::string& message) {
	std::map<const User *, privilege> users = channel.getUsersMap();
	std::map<const User *, privilege>::const_iterator it = users.begin();

	while (it != users.end()) {
		addResponse(resp, *(it->first), message);
		it++;
	}
}

void Server::sendMsgToChannel(std::map<User, std::string>* resp, const Channel& channel, Message* msg) {
	std::map<const User *, privilege> users = channel.getUsersMap();
	std::map<const User *, privilege>::const_iterator it = users.begin();
	std::string senderPrefix = ":" + msg->getSenderUser().getNick() + "!" + msg->getSenderUser().getName();
	std::string senderMessage = msg->getParams().back();


	while (it != users.end()) {
		if (it->first->getNick() != msg->getSenderUser().getNick())
			addResponse(resp, *(it->first), senderPrefix + " PRIVMSG " + channel.getName() + " :" + senderMessage);	
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

std::list<std::string> getRecieversFromInputList(std::string str) {
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

// -------------------------------- QUIT --------------------------------

std::map<User, std::string> Server::quitCommand(Message& msg) {
	std::map<User, std::string> resp;

	std::string userPrefix = ":" + msg.getSenderUser().getNick() + "!" + msg.getSenderUser().getName();
	if (msg.getParams().size() > 1) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 " + msg.getCommand() + " :Need more params");
	} else {
		std::map<std::string, Channel>::iterator chanIt = _channels.begin();
		std::string quitMessage = userPrefix + " QUIT";
		if (msg.getParams().size() == 1)
			quitMessage += " :" + msg.getParams().back();
		while (chanIt != _channels.end()) {
			if (chanIt->second.findUser(msg.getSenderUser()) != -1) {
				sendToChannel(&resp, chanIt->second, quitMessage);
			}
			chanIt++;
		}
		msg.getSenderUser().setDisconnect(true);
		addResponse(&resp, msg.getSenderUser(), quitMessage);
	}

	return resp;
}