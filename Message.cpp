#include "Message.hpp"

// -------------------- Constructors -------------------------


// -------------------- Getters and Setters -------------------------

// const std::string& Message::getPrefix() const {
// 	return _prefix;
// }

// void Message::setPrefix(const std::string& prefix) {
// 	_prefix = prefix;
// }

const std::string& Message::getCommand() const {
	return _command;
}

void Message::setCommand(const std::string& commandStr) {
	_command = commandStr;
}

const std::list<std::string>& Message::getParams() const {
	return _params;
}

void Message::setParams(const std::list<std::string>& params) {
	_params = params;
}

void Message::setSenderUser(User* user){
	_senderUser = user;
}

User& Message::getSenderUser() {
	return *_senderUser;
}

// -------------------- Methods -------------------------


// Order for parsing: 1. Get command, 2. Get params
Message Message::parseBuf(const std::string& buf) {
	Message message;
	std::string bufCopy = buf;
	// bufCopy = parsePrefix(bufCopy, message);
	bufCopy = parseCommand(bufCopy, message);
	parseParams(bufCopy, message);
	return message;
}

// !!!! Prefix not used for now !!!!!!
// std::string Message::parsePrefix(const std::string& buf, Message& msg) {
// 	std::string newBuf = buf;
// 	if (buf[0] == ':') {
// 		// Find and set the prefix from the buffer
// 		int end = buf.find(' ');
// 		msg.setPrefix(buf.substr(0, end));

// 		// Remove the prefix and space/s for the next function
// 		// Removing the prefix part
// 		newBuf.erase(0, end);

// 		//Find and remove the spaces (' ') after the prefix
// 		int start = newBuf.find_first_not_of(' ');
// 		newBuf.erase(0, start);
// 	}
// 	return newBuf;
// }

std::string Message::parseCommand(const std::string& buf, Message& msg) {
	std::string newBuf = buf;

	//Find and set the command
	int end = buf.find(' ');
	std::string tmp = buf.substr(0, end);
	std::string cmd;
	for (unsigned int i = 0; i < tmp.length(); i++) {
		cmd += toupper(tmp[i]);
	}
	msg.setCommand(cmd);

	// Remove the found command and the space/s after it
	newBuf.erase(0, end);
	int start = newBuf.find_first_not_of(' ');
	newBuf.erase(0, start);
	return newBuf;
}

void Message::parseParams(const std::string& buf, Message& msg) {
	std::string newBuf = buf;
	std::string normalParams;
	std::list<std::string> tmpList;
	size_t posOfParam;
	size_t colonPos = newBuf.find(':');
	normalParams = newBuf.substr(0, colonPos);
	while (!normalParams.empty()) {
		posOfParam = normalParams.find(' ');
		std::string tmp = normalParams.substr(0, posOfParam);
		if (!tmp.empty())
			tmpList.push_back(tmp);
		if (posOfParam != std::string::npos) 
			posOfParam++;
		normalParams.erase(0, posOfParam);
	}
	if (colonPos != std::string::npos)
		tmpList.push_back(newBuf.substr(colonPos + 1));
	msg.setParams(tmpList);
}


// ------------------- Operator overloads ---------------------
std::ostream& operator<<(std::ostream& stream, const Message& msg) {
	stream << "Message: {\n";
	stream << "\tcommand: [" << msg.getCommand() << "]\n";
	stream << "\tParams: { ";
	for (std::list<std::string>::const_iterator it = msg.getParams().begin(); it != msg.getParams().end(); it++) {
		stream << "[" << *it << "] ";
	}
	stream << "}\n}";

	return stream;
}