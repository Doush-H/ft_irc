#include "Message.hpp"

// -------------------- Constructors -------------------------


// -------------------- Getters and Setters -------------------------

const std::string& Message::getPrefix() const {
	return _prefix;
}

void Message::setPrefix(const std::string& prefix) {
	_prefix = prefix;
}

const std::string& Message::getCommandStr() const {
	return _commandStr;
}

void Message::setCommandStr(const std::string& commandStr) {
	_commandStr = commandStr;
}

int Message::getCommandInt() const {
	return _commandInt;
}

void Message::setCommandInt(int commandInt) {
	_commandInt = commandInt;
}

const std::list<std::string>& Message::getParams() const {
	return _params;
}

void Message::setParams(const std::list<std::string>& params) {
	_params = params;
}

bool Message::isIsCommandStr() const {
	return _isCommandStr;
}

void Message::setIsCommandStr(bool isCommandStr) {
	_isCommandStr = isCommandStr;
}

// -------------------- Methods -------------------------

Message Message::parseBuf(const std::string& buf) {
	Message message;
	parsePrefix(buf);
	parseCommand(buf);
	parseParams(buf);
	return message;
}

void Message::parsePrefix(const std::string& buf) {

}

void Message::parseCommand(const std::string& buf) {

}

void Message::parseParams(const std::string& buf) {

}
