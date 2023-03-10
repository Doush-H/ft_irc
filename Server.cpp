#include "Server.hpp"
#include "ChannelBot.hpp"

// ------------------ Getters and Setters ------------------

int Server::getPort() const {
	return _port;
}

void Server::setPort(int port) {
	Server::_port = port;
}

const std::string& Server::getPassword() const {
	return _password;
}

void Server::setPassword(const std::string& password) {
	Server::_password = password;
}

bool Server::isStop() const {
	return _stop;
}

void Server::setStop(bool stop) {
	_stop = stop;
}

int Server::getListeningSocket() const {
	return _listeningSocket;
}

void Server::setListeningSocket(int listeningSocket) {
	_listeningSocket = listeningSocket;
}

nfds_t Server::getActivePoll() const {
	return _activePoll;
}

void Server::setActivePoll(nfds_t pollSize) {
	_activePoll = pollSize;
}

// ---------------------- Constructors ------------------------

Server::Server(char** argv){
	// pthread_create(&_botManager, NULL, threadStart, &_channels);
	long port = strtol(argv[1], NULL, 0);
	if (port <= 0 || port > 65535 || errno == ERANGE)
		throw WrongPortNumberException();
	_port = (int) port;
	_password = argv[2];
	if (_password.empty() || checkForbidenChars(_password, " :,"))
		throw IncorrectPasswordException();
	_stop = false;
}

void	Server::botChecks() {
	std::map<std::string, Channel>::iterator it = _channels.begin();
	std::map<User, std::string> botResponses;
	while (it != _channels.end()) {
		// check for timestamps, add bots, remove bots

		struct timeval currentTime;
		gettimeofday(&currentTime, NULL);
		User* botUser = &it->second.channelBot.getBotUser();

		if (it->second.countUsers() >= 5 && !it->second.channelBot.getIsActive()) {

			it->second.initBotHistory();
			sendToChannel(&botResponses, it->second, ":" + botUser->getNick() + " JOIN " + it->second.getName());
			sendToChannel(&botResponses, it->second, ":" + botUser->getNick() + " MODE " + it->second.getName() + " +o " + botUser->getNick());
			it->second.addUser(*botUser, OPERATOR);
			it->second.channelBot.setIsActive(true);
			it->second.channelBot.getTimestamp().tv_sec = currentTime.tv_sec + BOT_TIMEOUT;
			refreshList(&botResponses);
			it++;
			continue;
		}

		if (it->second.countUsers() >= 6 && it->second.channelBot.getIsActive()) {
			it->second.channelBot.getTimestamp().tv_sec = currentTime.tv_sec + BOT_TIMEOUT;
			std::cout << "Bot active" << std::endl;
			it++;
			continue;
		}

		if (it->second.countUsers() < 6 && it->second.channelBot.getIsActive()) {
			std::cout << "Bot counting down" << std::endl;
			if (it->second.channelBot.getTimestamp().tv_sec <= currentTime.tv_sec) {
				it->second.channelBot.setIsActive(false);
				it->second.removeUser(*botUser);

				if (it->second.countUsers() == 0) {
					_channels.erase(it++->first);
					refreshList(&botResponses);
					continue;
				} else {
					sendToChannel(&botResponses, it->second, ":" + botUser->getNick() + " PART " + it->second.getName() + " :Bye bye :D");
				}
				
				std::cout << "Bot deleted" << std::endl;
				refreshList(&botResponses);
			}
		}
		it++;
	}
	// Send the bot responses
	sendResponse(&botResponses);
}

// ---------------------- Methods --------------------------

void Server::start() {
	try {
		setupSocket();
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		exit(-1);
	}
	int pollReturn;
	while (!_stop) {
		botChecks();
		// Waiting for some activity on any of the user fds
		pollReturn = poll(_userPoll, _activePoll, 5000); // poll timeout set to 5 seconds to reduce the amount of system calls

		// Check if poll failed
		if (pollReturn == -1)
			throw PollFailedException();
		unsigned int currentSize = _activePoll;

		// Loop the array and check if that fd is ready for reading
		for (unsigned int i = 0; i < currentSize; ++i) {
			try {
				// Checking if fd is has anything to read
				if (_userPoll[i].revents & POLLIN) {
					// If the fd is the socket fd there's a new connection, go and accept that
					if (_userPoll[i].fd == _listeningSocket)
						acceptUser();
					else
						executeCommand(i); // Fd is not the socket fd so it's a normal user, read and execute his request
				}
			} catch (std::exception& e) {
				// acceptUser() or executeCommand() failed so print the error
				std::cout << e.what() << std::endl;
			}
		}
	}
	// Server stopped so close the socket
	close(_listeningSocket);
}

void Server::setupSocket() {
	// Create a socket
	_listeningSocket = socket(PF_INET, SOCK_STREAM, 0);
	int optVal = 1;
	int optLen = sizeof(optVal);

	if (setsockopt(_listeningSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optVal, optLen) == -1 || setsockopt(_listeningSocket, SOL_SOCKET, SO_NOSIGPIPE, (const char*)&optVal, optLen) == -1) {
		std::cout << "setsockopt error" << std::endl;
		exit(-1);
	}

	fcntl(_listeningSocket, F_SETFL, O_NONBLOCK);
	if (_listeningSocket == -1) {
		throw FailedToCreateSocketException();
	}

	//Bind the socket to the IP / port
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(_port);

	inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);
	if (bind(_listeningSocket, (sockaddr *)&hint, sizeof(hint)) == -1) {
		throw FailedToBindSocket();
	}

	//Start listening from socket
	if (listen(_listeningSocket, SOMAXCONN) == -1) {
		throw FailedToListenOnSocket();
	}
	std::cout << "Listening on: " << _listeningSocket << std::endl;

	//Memset the poll
	memset(_userPoll, 0, sizeof(_userPoll));
	_activePoll = 1;
	_userPoll[0].fd	= _listeningSocket;
	_userPoll[0].events = POLLIN;
}

void Server::addToPoll(int fd) {
	if (_activePoll >= SOMAXCONN)
		throw ActivePollFullException();
	_userPoll[_activePoll].fd = fd;
	_userPoll[_activePoll].events = POLLIN;
	_activePoll++;
}

void Server::removeFromPoll(int userFd) {
	int i = -1;
	for (unsigned int j = 0; j < _activePoll; j++) {
		if (_userPoll[j].fd == userFd)
			i = j;
	}
	if (i == -1)
		throw IndexOutOfBoundException();
	_userPoll[i].fd = _userPoll[_activePoll - 1].fd;
	_userPoll[i].events = POLLIN;
	_userPoll[_activePoll - 1].fd = -1;
	_activePoll -= 1;
}

void Server::acceptUser() {
	sockaddr_in user;
	socklen_t userLen = sizeof(user);
	int connectedUserFd;
	connectedUserFd = accept(_listeningSocket, (sockaddr *)&user, &userLen);
	char	host[INET_ADDRSTRLEN];	// get user's hostmask
	inet_ntop(AF_INET, &user.sin_addr, host, INET_ADDRSTRLEN);
	std::string	hostmask(host);
    std::cout << "User hostmask: " << hostmask << std::endl;
	std::cout << "User connected on fd: " << connectedUserFd << std::endl;
	if (connectedUserFd == -1)
		throw FailedToAcceptConnectionException();
	std::cout << "Client connected" << std::endl;
	addToPoll(connectedUserFd);
	_users.insert(std::pair<int, User>(connectedUserFd, User(connectedUserFd, hostmask)));
}

void Server::executeCommand(int i) {
	char buf[MSG_LEN];
	memset(buf, 0, MSG_LEN);

	int userFd = _userPoll[i].fd;

	int recvBytes = recv(userFd, buf, MSG_LEN, 0);
	if (recvBytes == -1)
		throw ReadingTheMsgFailedException();
	if (recvBytes == 0) {
		std::cout << "Client disconnected" << std::endl;
		removeUser(_userPoll[i].fd);
	} else {
		// Get the user that we're reading from
		User* user = &_users.find(userFd)->second;

		//Append the current buffer to the buffer of the user
		user->extendBuffer(buf, recvBytes);

		std::string cmd;
		std::map<User, std::string> responses;


		// Going to execute commands until there's nothing to execute anymore
		while (!(cmd = user->getCommand()).empty()) {
			std::cout << "Going to execute: |" << cmd << "|" << std::endl;
			// Parse
			Message message = Message::parseBuf(cmd);
			message.setSenderUser(user);
			// Execute and get a response
			responses = commandCall(message);
			// Send back response
			sendResponse(&responses);
		}
	}
}

std::map<User, std::string> Server::commandCall(Message& msg) {
	std::map<User, std::string> response;
	if (msg.getCommand() == "PASS") {
		response = passCommand(msg);
	} else if (msg.getCommand() == "NICK") {
		response = nickCommand(msg);
	} else if (msg.getCommand() == "USER") {
		response = userCommand(msg);
	} else if (msg.getCommand() == "PING") {
		response = pingCommand(msg);
	} else if (msg.getCommand() == "CAP") {
		response = capCommand(msg);
	} else if (msg.getCommand() == "JOIN") {
		response = joinCommand(msg);
	} else if (msg.getCommand() == "PRIVMSG") {
		response = privmsgCommand(msg);
	} else if (msg.getCommand() == "TOPIC") {
		response = topicCommand(msg);
	} else if (msg.getCommand() == "WHO") {
		response = whoCommand(msg);
	} else if (msg.getCommand() == "PART") {
		response = partCommand(msg);
	} else if (msg.getCommand() == "MODE") {
		response = modeCommand(msg);
	} else if (msg.getCommand() == "QUIT") {
		response = quitCommand(msg);
	} else if (msg.getCommand() == "LIST") {
		response = listCommand(msg);
	} else if (msg.getCommand() == "INVITE") {
		response = inviteCommand(msg);
	} else if (msg.getCommand() == "KICK") {
		response = kickCommand(msg);
	} else {
		addResponse(&response, msg.getSenderUser(), SERV_PREFIX "421 " + msg.getCommand() + " :" + msg.getCommand());
	}
	return response;
}

void Server::sendResponse(std::map<User, std::string>* responses) {
	std::map<User, std::string>::iterator it = responses->begin();
	while (it != responses->end()) {
		it->second.append("\r\n");
		// dont throw just print error and continue
		if (send(it->first.getUserFd(), it->second.c_str(), it->second.length(), 0) == -1) {
			std::cout << "Couldn't send the response to fd: " << it->first.getUserFd() << std::endl;
			it++;
			continue;
		}

		std::cout << "|" << it->second << "| was successfully sent to " + it->first.getNick() + "\n" << std::endl;
		if (it->first.isDisconnect()) {
			removeUser(it->first.getUserFd());
			std::cout << it->first.getNick() << " QUIT THE SERVER" << std::endl;
		}
		it++;
	}
}

void Server::removeUser(int fd) {
	removeUserFromChannels(_users.find(fd)->second);
	_users.erase(fd);
	close(fd);
	try {
		removeFromPoll(fd);
	} catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
		exit(1);
	}
}

void Server::removeUserFromChannels(const User& user) {
	std::map<std::string, Channel>::iterator it = _channels.begin();
	while (it != _channels.end()) {
		std::map<const User*, privilege>::const_iterator userIt = it->second.getUsersMap().find(&user);
		if (userIt != it->second.getUsersMap().end())
			it->second.removeUser(user);
		it++;
	}
}
