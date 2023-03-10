#include "User.hpp"

User::User() {
	_allowConnection = false;
	_isRegistered = false;
	_disconnect = false;
	_userFd = -1;
	_hostmask = "127.0.0.1";
	_nick = "*";
	_name = "*";
	_fullName = "*";
}

User::User(int fd, std::string hostmask) {
	_allowConnection = false;
	_isRegistered = false;
	_disconnect = false;
	_userFd = fd;
	_hostmask = hostmask;
	_nick = "*";
	_name = "*";
	_fullName = "*";
}

User::User(const User& other) {
	_allowConnection = other.isAllowConnection();
	_isRegistered = other.isRegistered();
	_userFd = other.getUserFd();
	_nick = other.getNick();
	_name = other.getName();
	_disconnect = other.isDisconnect();
	_fullName = other.getFullName();
}

//------------------ Operator overloads --------------------
bool User::operator<(const User& other) const{
	return (_userFd < other.getUserFd());
}

//------------------ Getters and Setters --------------------

bool User::isDisconnect() const {
	return _disconnect;
}

void User::setDisconnect(bool disconnect) {
	_disconnect = disconnect;
}

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

const std::string& User::getHostmask() const {
	return _hostmask;
}

bool User::isRegistered() const {
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

void User::setHostmask(std::string str) {
	User::_hostmask = str;
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
