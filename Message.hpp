#ifndef MESSAGE_HPP
#define MESSAGE_HPP

# include <string>
# include <list>
# include "User.hpp"
# include <iterator>
# include <iostream>

class Message {
private:
	// !!!!! Prefix probably will not be needed so it's not going to be used for now!!!!
	// std::string 				_prefix;
	std::string 				_command;
	std::list<std::string>		_params;
	User*						_senderUser;

public:

	// ------------------ Getters and setters ------------------
	// const std::string& getPrefix() const;
	// void setPrefix(const std::string& prefix);
	const std::string& 					getCommand() const;
	void 								setCommand(const std::string& commandStr);
	const std::list<std::string>& 		getParams() const;
	void 								setParams(const std::list<std::string>& params);
	void								setSenderUser(User* user);
	User&								getSenderUser();

	// ------------------- Methods -------------------
	static Message parseBuf(const std::string& buf);
private:
	// static std::string parsePrefix(const std::string& buf, Message& msg);
	static std::string 		parseCommand(const std::string& buf, Message& msg);
	static void 			parseParams(const std::string& buf, Message& msg);
};

std::ostream& operator<<(std::ostream& stream, const Message& msg);


#endif //MESSAGE_HPP
