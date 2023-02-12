#include "Server.hpp"

// ------------------------------- Helper Functions ----------------------------------

void Server::addResponse(std::map<User, std::string>* resp, const User& receiver, const std::string& respMessage) {
	
	std::pair<std::map<User, std::string>::iterator, bool> insertResult = resp->insert(std::pair<User, std::string>(receiver, respMessage));
	if (insertResult.second == false) {
		std::map<User, std::string>::iterator it = insertResult.first;
		it->second.append("\r\n" + respMessage);
	}
}

void Server::sendToChannel(std::map<User, std::string>* resp, Channel& channel, const std::string& message) {
	std::map<const User *, privilege> users = channel.getUsersMap();
	std::map<const User *, privilege>::iterator it = users.begin();

	while (it != users.end()) {
		if (it->first->getUserFd() == -1) {
			it++;
			continue ;
		}
		addResponse(resp, *(it->first), message);
		it++;
	}
}

void Server::sendMsgToChannel(std::map<User, std::string>* resp, Channel& channel, Message* msg) {
	std::map<const User *, privilege> users = channel.getUsersMap();
	std::map<const User *, privilege>::iterator it = users.begin();
	std::string senderPrefix = ":" + msg->getSenderUser().getNick() + "!" + msg->getSenderUser().getName();
	std::string senderMessage = msg->getParams().back();

	while (it != users.end()) {
		if (it->first->getUserFd() == -1) {
			it++;
			continue ;
		}
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
	if (msg.getSenderUser().getFullName().compare("*") && msg.getSenderUser().getNick().compare("*") && msg.getSenderUser().isAllowConnection() && !msg.getSenderUser().isRegistered()) {
		addResponse(resp, msg.getSenderUser(), SERV_PREFIX "001 " + msg.getSenderUser().getNick() + " :Welcome to our irc server " + msg.getSenderUser().getNick());
		msg.getSenderUser().setIsRegistered(true);
		listChannels(resp, &msg, "");
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

void	Server::refreshList(std::map<User, std::string>* resp)
{
	Message	relist;
	relist.setCommand("LIST");
	relist.setParams(std::list<std::string> ());
	std::map<int, User>::iterator	it = _users.begin();
	for (; it != _users.end(); it++)
	{
		relist.setSenderUser(&it->second);
		listChannels(resp, &relist, "");
	}
}

bool Server::checkForbidenChars(const std::string& str, const std::string& chars) {
	for (unsigned int i = 0; i < chars.size(); i++) {
		if (str.find(chars[i]) != std::string::npos)
			return true;
	}
	return false;
}
