#ifndef USER_HPP
#define USER_HPP
# include <string>

class User {
private:
	std::string 	_buf;
	std::string 	_nick;
	std::string 	_name;
	std::string 	_fullName;
	bool 			_isRegistered;
	bool 			_allowConnection;
	int 			_userFd;

public:
	User(int fd);

	//--------------- Getters and Setters ------------------
	const std::string& getBuf() const;
	void setBuf(const std::string& buf);
	const std::string& getNick() const;
	void setNick(const std::string& nick);
	const std::string& getName() const;
	void setName(const std::string& name);
	const std::string& getFullName() const;
	void setFullName(const std::string& fullName);
	bool isRegistered() const;
	void setIsRegistered(bool isRegistered);
	bool isAllowConnection() const;
	void setAllowConnection(bool allowConnection);
	int getUserFd() const;
	void setUserFd(int userFd);

	// ----------------- Methods ---------------------
	
	// Returns the next command from users buffer if found, else returns empty string 
	std::string getCommand();

	// Extends the users buffer
	void extendBuffer(char buff[], size_t size);
};


#endif //USER_HPP
