#include "Server.hpp"

// -------------------------------- PASS --------------------------------

std::map<User, std::string> Server::passCommand(Message& msg){
	std::map<User, std::string> resp;
	if (msg.getParams().size() != 1) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 " + msg.getSenderUser().getNick() + " " + msg.getCommand() + " :Wrong arguments for the command");
	} else if (msg.getSenderUser().isRegistered()) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 " + msg.getSenderUser().getNick() + " :You're already registered");
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
	if (msg.getParams().size() != 1 || checkForbidenChars(msg.getParams().front(), "[,]{^}!:*")) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 " + msg.getSenderUser().getNick() + " " + msg.getCommand() + " :Wrong arguments for the command");
	} else if (!msg.getSenderUser().isAllowConnection()) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 " + msg.getSenderUser().getNick() + " :Please provide the server password with PASS command before registration");
	} else {
		std::list<std::string> params = msg.getParams();
		std::string inputNick = params.front();
		std::map<int, User>::iterator it = _users.begin();

		bool setName = true;
		while (it != _users.end()) {
			if (it->second.getNick() == inputNick) {
				setName = false;
				addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "433 " + msg.getSenderUser().getNick() + " " + inputNick + " :Nickname is already in use");
			}
			it++;
		}
		if (setName) {
			std::string userPrefix = ":" + msg.getSenderUser().getNick() + "!" + msg.getSenderUser().getName();
			addResponse(&resp, msg.getSenderUser(), userPrefix + " NICK " + inputNick);
			std::map<std::string, Channel>::iterator chanIt = _channels.begin();
			while (chanIt != _channels.end()) {
				if (chanIt->second.findUser(msg.getSenderUser()) != -1) {
					sendToChannel(&resp, chanIt->second, userPrefix + " NICK " + inputNick);
				}
				chanIt++;
			}
			msg.getSenderUser().setNick(inputNick);
		}
	}
	checkIfRegistered(msg, &resp);
	return resp;
}

// -------------------------------- USER -------------------------------

std::map<User, std::string> Server::userCommand(Message& msg){
	std::map<User, std::string> resp;
	if (msg.getParams().size() != 4 || checkForbidenChars(msg.getParams().front(), "[,]{^}!:")) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 " + msg.getSenderUser().getNick() + " " + msg.getCommand() + " :Wrong arguments for the command");
	} else if (!msg.getSenderUser().isAllowConnection()) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 " + msg.getSenderUser().getNick() + " :Please provide the server password with PASS command before registration");
	} else if (msg.getSenderUser().isRegistered()) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 " + msg.getSenderUser().getNick() + " :You can not change your user details after registration");
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