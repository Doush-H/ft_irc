#ifndef MESSAGE_HPP
#define MESSAGE_HPP

# include <string>
# include <list>

class Message {
private:
	std::string 				_prefix;
	std::string 				_commandStr;
	int 						_commandInt;
	std::list<std::string>		_params;
	bool 						_isCommandStr;

public:

	// ------------------ Getters and setters ------------------
	const std::string& getPrefix() const;
	void setPrefix(const std::string& prefix);
	const std::string& getCommandStr() const;
	void setCommandStr(const std::string& commandStr);
	int getCommandInt() const;
	void setCommandInt(int commandInt);
	const std::list<std::string>& getParams() const;
	void setParams(const std::list<std::string>& params);
	bool isIsCommandStr() const;
	void setIsCommandStr(bool isCommandStr);

	// ------------------- Methods -------------------
	static Message parseBuf(const std::string& buf);
private:
	static void parsePrefix(const std::string& buf);
	static void parseCommand(const std::string& buf);
	static void parseParams(const std::string& buf);
};


#endif //MESSAGE_HPP
