#include "Server.hpp"

int Server::getPort() const {
	return _port;
}

// ------------------ Getters and Setters ------------------

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
	long port = strtol(argv[1], nullptr, 0);
	if (port < 0 || errno == ERANGE)
		throw WrongPortNumberException();
	_port = (int) port;
	_password = argv[2];
	_stop = false;
}

// ---------------------- Methods --------------------------

void Server::start() {
	try {
		setupSocket();
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		exit (-1);
	}
	int pollReturn;
	while (!_stop) {
		// Waiting for some activity on any of the user fds
		pollReturn = poll(_userPoll, _activePoll, -1);

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
	_listeningSocket = socket(PF_INET, SOCK_STREAM, getprotobyname("tcp")->p_proto);
	int optVal = 1;
	int optLen = sizeof(optVal);
	if (setsockopt(_listeningSocket, SOL_SOCKET,  SO_REUSEADDR, (const char*)&optVal, optLen) == -1) {
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

void Server::removeFromPoll(int i) {
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
	if (connectedUserFd == -1)
		throw FailedToAcceptConnectionException();
	std::cout << "Client connected" << std::endl;
	addToPoll(connectedUserFd);
	_users.insert(std::pair<int, User>(connectedUserFd, User(connectedUserFd)));
}

void Server::executeCommand(int i) {
	char buf[MSG_LEN];
	memset(buf, 0, MSG_LEN);

	int userFd = _userPoll[i].fd;

	int recvBytes = recv(userFd, buf, MSG_LEN, 0);
	if (recvBytes == -1)
		throw ReadingTheMsgFailedException();
	if (recvBytes == 0) {
		// !!!!!!! Probably need to close the disconnected users fd too, not sure tho :D !!!!!!!!
		std::cout << "Client disconnected" << std::endl;
		removeUser(i);
	} else {
		// Get the user that we're reading from
		User* user = &_users.find(userFd)->second;

		//Append the current buffer to the buffer of the user
		user->extendBuffer(buf, recvBytes);

		std::string cmd;
		std::string response;

		// Going to execute commands until there's nothing to execute anymore
		while (!(cmd = user->getCommand()).empty()) {
			std::cout << "Going to execute: |" << cmd << "|" << std::endl;
			// Parse
			Message message = Message::parseBuf(cmd);
			message.setSenderUser(user);
			// Execute and get a response
			response = commandCall(message);
			// Send back response
			try {
				sendResponse(response, userFd);
			} catch (std::exception& e) {
				std::cout << e.what() << std::endl;
			}
		}
	}
}

std::string Server::commandCall(Message& msg) {
	std::string response;
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
	} else {
		response = SERV_PREFIX "421 " + msg.getCommand() + " :" + msg.getCommand();
	}
	response += "\r\n";
	return response;
}

void Server::sendResponse(const std::string& response, int userFd) {
	ssize_t sendBytes = send(userFd, response.c_str(), response.length(), 0);
	if (sendBytes == -1)
		throw SendingTheMsgFailedException();
	std::cout << "|" << response << "| was successfully sent to the user" << std::endl;
}

void Server::removeUser(int i) {
	if (i <= 0 || i >= SOMAXCONN)
		throw IndexOutOfBoundException();
	_users.erase(_userPoll[i].fd);
	removeFromPoll(i);
}
