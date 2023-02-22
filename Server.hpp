#ifndef SERVER_HPP
#define SERVER_HPP
# include <string>
# include <cstdlib>
# include <list>
# include <sys/poll.h>
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <cerrno>
# include <iostream>
# include <fcntl.h>
# include <unistd.h>
# include <map>
# include <iterator>
# include "User.hpp"
# include "Message.hpp"
# include "Channel.hpp"
# include <cstring>
# include <sstream>
# include <pthread.h>

# include <sys/types.h>
# include <netinet/in.h>

# define MSG_LEN 4096
# define SERV_PREFIX ":42irc.com "

class Server {

private:
	int 					_port;					// Port on which the server/socket is listening
	std::string				_password;				// Pass required to connect to the server
	pollfd					_userPoll[SOMAXCONN]; 	// Poll with all the users
	nfds_t 					_activePoll; 			// The number of user that we have in poll
	bool					_stop;					// Tells the server to stop if set true
	int 					_listeningSocket; 		// Fd of the socket that was creating, it's used for accepting new connection
	std::map<int, User>		_users;					// A map with fds as keys and User that the fd belongs to as the value of the map
	std::map<std::string, Channel>	_channels;		// Private channels are displayed in the private section of the client list

public:
	// -------------------- Exceptions ---------------------
class WrongPortNumberException : public std::exception {
		const char * what() const throw() {
			return "Provided port number was incorrect";
		}
	};

class FailedToAcceptConnectionException : public std::exception {
		const char * what() const throw() {
			return "Failed to accept a user trying to connect";
		}
	};

class FailedToCreateSocketException : public std::exception {
		const char * what() const throw() {
			return "Failed to create a socket";
		}
	};

class FailedToBindSocket : public std::exception {
		const char * what() const throw() {
			return "Failed to bind the socket to IP/port";
		}
	};

class FailedToListenOnSocket : public std::exception {
		const char * what() const throw() {
			return "Failed to listen on the socket";
		}
	};

class ActivePollFullException : public std::exception {
		const char * what() const throw() {
			return "Number of connection is full, can't accept more connections";
		}
	};

class IndexOutOfBoundException : public std::exception {
		const char * what() const throw() {
			return "Given index is out of bounds";
		}
	};

class PollFailedException : public std::exception {
		const char * what() const throw() {
			return "poll() failed";
		}
	};

class ReadingTheMsgFailedException : public std::exception {
		const char * what() const throw() {
			return "Failed to read the input from the user";
		}
	};

class SendingTheMsgFailedException : public std::exception {
		const char * what() const throw() {
			return "Failed to send a response to the user";
		}
	};

class EmptyPasswordException : public std::exception {
		const char * what() const throw() {
			return "Empty password not allowed";
		}
	};

	// -------------------- Constructors ----------------------
	Server(char** argv);

	// ---------------- Getters and Setters ---------------------
	int 									getPort() const;
	void 									setPort(int port);
	const 									std::string& getPassword() const;
	void 									setPassword(const std::string& password);
	bool 									isStop() const;
	void 									setStop(bool stop);
	int 									getListeningSocket() const;
	void 									setListeningSocket(int listeningSocket);
	nfds_t 									getActivePoll() const;
	void 									setActivePoll(nfds_t pollSize);

	// --------------------- Methods ------------------------

	// Starts the server and contains the main loop
	void 									start();

	// Contains the setup of the server
	void 									setupSocket();

	// Accepts new users
	void 									acceptUser();

	// Reads , parses and responds to commands from user with index i in the poll array
	void 									executeCommand(int i);

	// Removes a user from user map and poll array (should probably close the fds too)
	void 									removeUser(int fd);
	void 									removeUserFromChannels(const User& user);

	// -------------------- Poll handling --------------------

	// Adds the new users to poll array
	void 									addToPoll(int fd);

	// Removes user with index i from the poll array
	void 									removeFromPoll(int i);

	// Find the correct command and call it
	std::map<User, std::string>				commandCall(Message& msg);

	// Send the response back to the user
	void									sendResponse(std::map<User, std::string>* responses);

	// ----------------------- Util commands ---------------------------

	/// @param resp the map to which you want to add a new response
	/// @param reciever reciever of the new response
	/// @param respMessage the message that's gonna be sent to the reciever
	void									addResponse(std::map<User, std::string>* resp, const User& receiver, const std::string& respMessage);

	// Returns an interator to the found user else returns an iterator pointing to _users.end()
	std::map<int, User>::iterator			findUserByNick(std::string nickName);

	/// @param resp the response map to which the responses will be added
	/// @param channel the channel to which the message will be sent
	/// @param message the message that will be sent to the channel
	void 									sendToChannel(std::map<User, std::string>* resp, const Channel& channel, const std::string& message);

	// sends a PRIVMSG to poeple in the channel excluding the sender User
	void 									sendMsgToChannel(std::map<User, std::string>* resp, const Channel& channel, Message* msg);

	// checks if the user who sent the message is registered, if yes then it will add the welcome message
	void									checkIfRegistered(Message& msg, std::map<User, std::string>* resp);

	// Check if the given parameter containss any character from chars string, if yes returns true else returns false
	bool									checkForbidenChars(const std::string& str, const std::string& chars);

	// -------------------- Commands (everyone of them will return the response that they generated) --------------------
	// -------------------- The return from command functions is a map now in order to have the ability to send messages to multiple people at the same time --------------------

	std::map<User, std::string>				passCommand(Message& msg);
	std::map<User, std::string> 			nickCommand(Message& msg);
	std::map<User, std::string> 			userCommand(Message& msg);
	std::map<User, std::string>				pingCommand(Message& msg);
	std::map<User, std::string>				capCommand(Message& msg);
	std::map<User, std::string>				joinCommand(Message& msg);
	std::map<User, std::string>				topicCommand(Message& msg);
	std::map<User, std::string>				privmsgCommand(Message& msg);
	std::map<User, std::string>				whoCommand(Message& msg);
	std::map<User, std::string>				modeCommand(Message& msg);
	std::map<User, std::string>				partCommand(Message& msg);
	std::map<User, std::string>				quitCommand(Message& msg);
	std::map<User, std::string>				listCommand(Message& msg);
	std::map<User, std::string>				inviteCommand(Message& msg);
	std::map<User, std::string>				kickCommand(Message& msg);

	void									botChecks();
	void									refreshList(std::map<User, std::string>* resp);
	// A helper funnction to be used by other functions (including listCommand()), pass the channels param as an empty string in order to list all channels
	void 									listChannels(std::map<User, std::string>* resp, Message* msg, std::string channels);
	void									sendInfoToNewJoin(Message& msg, const Channel* channel, std::map<User, std::string>* resp);
	void 									whoEveryone(std::map<User, std::string>* resp, Message* msg, const std::string& mask);
	void 									whoChannel(std::map<User, std::string>* resp, Message* msg, const Channel& channel, int priv);
	void 									whoOneParam(std::map<User, std::string>* resp, Message* msg);
	void 									whoTwoParam(std::map<User, std::string>* resp, Message* msg);
	void									modeReturnFlags(std::map<User, std::string> *resp, Message &msg);
	void									modeChangeChannel(std::map<User, std::string> *resp, Message &msg);
	void									modeChangeChannelUser(std::map<User, std::string> *resp, Message &msg);
	void 									kickNoComment(std::map<User, std::string>* resp, Message msg, Channel &chan, User &rm);
	void 									kickComment(std::map<User, std::string>* resp, Message msg, Channel &chan, User &rm);
	void									spawnBot(std::map<User, std::string> *resp, Channel &chan, std::string chanName);
	privilege								checkPrivilege(Message& msg, Channel &chan, std::map<User, std::string>* resp);

	//If no errors occurred return true, else returns false
	bool						 			privmsgToUserCommand(Message* msg, std::map<User, std::string>* resp, const std::string& userNick);

	//If no errors occurred return true, else returns false
	bool 									privmsgToChannelCommand(Message* msg, std::map<User, std::string>* resp, const std::string& chanName);
};

// ------------ Helper functions ------------

std::list<std::string> getRecieversFromInputList(std::string str);
bool containsMask(const char* mask, const char* str);

#endif //SERVER_HPP
