#include "Server.hpp"

void Server::checkIfRegistered(Message& msg, std::string& resp) {
	if (!msg.getSenderUser().getFullName().empty() && !msg.getSenderUser().getNick().empty() && msg.getSenderUser().isAllowConnection() && !msg.getSenderUser().isRegistered()) {
		resp = SERV_PREFIX "001 " + msg.getSenderUser().getNick() + " :Welcome to our irc server " + msg.getSenderUser().getNick();
		msg.getSenderUser().setIsRegistered(true);
	}
}

std::string Server::passCommand(Message& msg){
	std::string resp;
	if (msg.getParams().size() != 1) {
		resp = SERV_PREFIX "461 :Wrong number of parameters";
	} else if (msg.getSenderUser().isRegistered()) {
		resp = SERV_PREFIX "462 :You're already registered";
	} else {
		std::list<std::string> params = msg.getParams();
		std::string inputPass = params.front();
		if (inputPass == _password) {
			msg.getSenderUser().setAllowConnection(true);
		} else {
			msg.getSenderUser().setAllowConnection(false);
			resp = SERV_PREFIX "464 * :Password incorrect";
		}
	}
	return resp;
}

std::string Server::nickCommand(Message& msg){
	std::string resp;
	if (msg.getParams().size() != 1) {
		resp = SERV_PREFIX "431 :No nick name was given";
	} else if (!msg.getSenderUser().isAllowConnection()) {
		resp = SERV_PREFIX "462 :Please provide the server password with PASS command before registration";
	} else {
		std::list<std::string> params = msg.getParams();
		std::string inputNick = params.front();
		std::map<int, User>::iterator it = _users.begin();
		std::map<int, User>::iterator eit = _users.end();

		bool setName = true;
		while (it != eit) {
			if (it->second.getNick() == inputNick) {
				setName = false;
				resp = SERV_PREFIX "433 " + inputNick + " :Nickname is already in use";
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
		resp = SERV_PREFIX "461 :Not all parameters were provided";
	} else if (!msg.getSenderUser().isAllowConnection()) {
		resp = SERV_PREFIX "462 :Please provide the server password with PASS command before registration";
	} else if (msg.getSenderUser().isRegistered()) {
		resp = SERV_PREFIX "462 :You can not change your user details after registration";
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

std::string Server::pingCommand(Message& msg){
	std::string resp;
	if (msg.getParams().size() < 1 || msg.getParams().size() > 2) {
		resp = SERV_PREFIX "461 :Wrong number of parameters";
	} else {
		std::list<std::string> params = msg.getParams();
		resp = "PONG :" + params.front();
	}
	return resp;
}

std::string Server::capCommand(Message& msg){
	std::string resp;
	if (msg.getParams().front() == "LS")
		resp = "CAP * LS :End of CAP LS negotiation";
	return resp;
}

std::string Server::joinCommand(Message& msg){
	std::string resp;

	if (msg.getParams().size() < 1 || msg.getParams().size() > 2) {
		return (SERV_PREFIX "461 :Not all parameters were provided");
	} else if (!msg.getSenderUser().isAllowConnection() || !msg.getSenderUser().isAllowConnection()) {
		return (SERV_PREFIX "462 :Please log in before joining any channels");
	} else {
		std::list<std::string> params = msg.getParams();
		std::string chanName = params.front();
		std::map<std::string, Channel>::iterator	chan = _channels.find(chanName);
		if (chan != _channels.end())
		{
			std::string key = params.back();
			if (msg.getParams().size() == 2 && chan->second.getChannelKey() == key)
				chan->second.addUser(msg.getSenderUser(), NO_PRIO);
			else if (msg.getParams().size() == 2)
				return (SERV_PREFIX "475 :Cannot join channel, invalid key");
			else
			{
				chan->second.addUser(msg.getSenderUser(), NO_PRIO);
				return (SERV_PREFIX "JOIN " + chanName + "\r\n:" + msg.getSenderUser().getNick() + "!" \
					+ msg.getSenderUser().getName() + "@42irc.com JOIN :" + chanName);
			}
		}
		else
		{
			Channel	newchan	= Channel(chanName, 0);
			newchan.addUser(msg.getSenderUser(), OPERATOR);
			_channels.insert(std::pair<std::string, Channel>(chanName, newchan));
			return (SERV_PREFIX "JOIN " + chanName + "\r\n:" + msg.getSenderUser().getNick() + "!" \
					+ msg.getSenderUser().getName() + "@42irc.com JOIN :" + chanName);
		}
	}
	return resp;
}