#include "Server.hpp"

bool    containsMask(const char* mask, const char* str);

// -------------------------------- WHO --------------------------------

std::map<User, std::string>	Server::whoCommand(Message& msg)
{
	std::map<User, std::string> resp;
	std::list<std::string> msgParams = msg.getParams();

	if (msg.getParams().size() > 2) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "461 " + msg.getSenderUser().getNick() + " " + msg.getCommand() + " :Not all parameters were provided");
	} else if (!msg.getSenderUser().isRegistered()) {
		addResponse(&resp, msg.getSenderUser(), SERV_PREFIX "462 " + msg.getSenderUser().getNick() + " :Please register");
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
		if (mask.empty() || containsMask(mask.c_str(), msg->getSenderUser().getNick().c_str()))
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

	if (maskParam.at(0) == '#' || maskParam.at(0) == '&') { // If channel who execute whoChannel
		std::map<std::string, Channel>::const_iterator it = _channels.find(maskParam);
		if (it == _channels.end()) { // if the channel was not found return error
			addResponse(resp, msg->getSenderUser(), SERV_PREFIX "403 " + msg->getSenderUser().getNick() \
				+ " " + maskParam + " :No such channel");
		} else { // else send the list of people in the channel with all privileges
			whoChannel(resp, msg, it->second, -1);
		}
	} else { // if not channel who execute whoEveryone
		whoEveryone(resp, msg, maskParam);
	}
}

void Server::whoTwoParam(std::map<User, std::string>* resp, Message* msg) {
	std::string maskParam = msg->getParams().front();
	std::string secondParam = msg->getParams().back();

	if (secondParam != "o") {
		addResponse(resp, msg->getSenderUser(), SERV_PREFIX "461 " + msg->getSenderUser().getNick() + " " + msg->getCommand() + " :Wrong parameters for the command");
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
