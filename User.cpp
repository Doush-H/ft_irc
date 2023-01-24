#include "User.hpp"

User::User(int fd) {
	_allowConnection = false;
	_isRegistered = false;
	_userFd = fd;
}

//------------------ Getters and Setters --------------------

const std::string& User::getBuf() const {
	return _buf;
}

void User::setBuf(const std::string& buf) {
	User::_buf = buf;
}

const std::string& User::getNick() const {
	return _nick;
}

void User::setNick(const std::string& nick) {
	User::_nick = nick;
}

const std::string& User::getName() const {
	return _name;
}

void User::setName(const std::string& name) {
	User::_name = name;
}

const std::string& User::getFullName() const {
	return _fullName;
}

void User::setFullName(const std::string& fullName) {
	User::_fullName = fullName;
}

bool User::isRegistered1() const {
	return _isRegistered;
}

void User::setIsRegistered(bool isRegistered) {
	User::_isRegistered = isRegistered;
}

bool User::isAllowConnection() const {
	return _allowConnection;
}

void User::setAllowConnection(bool allowConnection) {
	User::_allowConnection = allowConnection;
}

int User::getUserFd() const {
	return _userFd;
}

void User::setUserFd(int userFd) {
	User::_userFd = userFd;
}

// ------------------ Methods --------------------

//if there's a cmd to execute returns the cmd else return empty string.
std::string User::getCommand() {
	std::string ret;

	size_t posCR = _buf.find('\r');
	size_t posFL = _buf.find('\n');

	if (posCR != std::string::npos && posFL != std::string::npos && (posCR + 1) == posFL) {
		ret = _buf.substr(0, posCR);
		_buf.erase(0, posFL + 1);
	}
	return ret;

}

void User::extendBuffer(char* buf, size_t size) {
	_buf.append(std::string(buf, 0, size));
}
