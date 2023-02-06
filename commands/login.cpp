#include "Server.hpp"

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

// -------------------------------- USER -------------------------------

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