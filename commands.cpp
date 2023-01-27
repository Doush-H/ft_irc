#include "Server.hpp"

void Server::checkIfRegistered(Message& msg, std::string& resp) {
	if (!msg.getSenderUser().getFullName().empty() && !msg.getSenderUser().getNick().empty() && msg.getSenderUser().isAllowConnection() && !msg.getSenderUser().isRegistered()) {
		resp = ":42irc.com 001 " + msg.getSenderUser().getNick() + " :Welcome to our irc server " + msg.getSenderUser().getNick();
		msg.getSenderUser().setIsRegistered(true);
	}
}


std::string Server::passCommand(Message& msg){
	std::string resp;
	if (msg.getParams().size() != 1) {
		resp = ":42irc.com 461 :Wrong number of parameters";
	} else if (msg.getSenderUser().isRegistered()) {
		resp = ":42irc.com 462 :You're already registered";
	} else {
		std::list<std::string> params = msg.getParams();
		std::string inputPass = params.front();
		if (inputPass == _password) {
			msg.getSenderUser().setAllowConnection(true);
		} else {
			msg.getSenderUser().setAllowConnection(false);
			resp = ":42irc.com ERROR :Password incorrect";
		}
	}
	return resp;
}

std::string Server::nickCommand(Message& msg){
	std::string resp;
	if (msg.getParams().size() != 1) {
		resp = ":42irc.com 431 :No nick name was given";
	} else if (!msg.getSenderUser().isAllowConnection()) {
		resp = ":42irc.com 462 :Please provide the server password with PASS command before registration";
	} else {
		std::list<std::string> params = msg.getParams();
		std::string inputNick = params.front();
		std::map<int, User>::iterator it = _users.begin();
		std::map<int, User>::iterator eit = _users.end();

		bool setName = true;
		while (it != eit) {
			if (it->second.getNick() == inputNick) {
				setName = false;
				resp = ":42irc.com 433 " + inputNick + " :Nickname is already in use";
			}
			it++;
		}
		if (setName)
			msg.getSenderUser().setNick(inputNick);
	}
	checkIfRegistered(msg, resp);
	return resp;
}

std::string Server::userCommand(Message& msg){
	std::string resp;
	if (msg.getParams().size() != 4) {
		resp = ":42irc.com 461 :Not all parameters were provided";
	} else if (!msg.getSenderUser().isAllowConnection()) {
		resp = ":42irc.com 462 :Please provide the server password with PASS command before registration";
	} else if (msg.getSenderUser().isRegistered()) {
		resp = ":42irc.com 462 :You can not change your user details after registration";
	} else {
		std::list<std::string> params = msg.getParams();
		std::string inputName = params.front();
		std::string inputFullName = params.back();
		msg.getSenderUser().setName(inputName);
		msg.getSenderUser().setFullName(inputFullName);
	}
	checkIfRegistered(msg, resp);
	return resp;
}
