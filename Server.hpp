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
	std::map<int, User>		_users; 				// A map with fds as keys and User that the fd belongs to as the value of the map
	// may change this
	std::map<int, Channel>	_normal_channels;		// Normal channels are neither secret or private
	std::map<int, Channel>	_secret_channels;		// Secret channels are not displayed on the client list
	std::map<int, Channel>	_private_channels;		// Private channels are displayed in the private section of the client list
	// 

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

	// -------------------- Constructors ----------------------
	Server(char** argv);

	// ---------------- Getters and Setters ---------------------
	int 			getPort() const;
	void 			setPort(int port);
	const 			std::string& getPassword() const;
	void 			setPassword(const std::string& password);
	bool 			isStop() const;
	void 			setStop(bool stop);
	int 			getListeningSocket() const;
	void 			setListeningSocket(int listeningSocket);
	nfds_t 			getActivePoll() const;
	void 			setActivePoll(nfds_t pollSize);

	// --------------------- Methods ------------------------

	// Starts the server and contains the main loop
	void 					start();

	// Contains the setup of the server
	void 					setupSocket();

	// Accepts new users
	void 					acceptUser();

	// Reads , parses and responds to commands from user with index i in the poll array
	void 					executeCommand(int i);

	// Removes a user from user map and poll array (should probably close the fds too)
	void 					removeUser(int i);

	// -------------------- Poll handling --------------------

	// Adds the new users to poll array
	void 					addToPoll(int fd);

	// Removes user with index i from the poll array
	void 					removeFromPoll(int i);

	// Find the correct command and call it
	std::string				commandCall(Message& msg);

	// Send the response back to the user
	void					sendResponse(const std::string& response, int userFd);

	// -------------------- Commands (everyone of them will return the response that they generated) --------------------

	std::string				passCommand(Message& msg);
	std::string 			nickCommand(Message& msg);
	std::string 			userCommand(Message& msg);
	std::string				pingCommand(Message& msg);
	std::string				capCommand(Message& msg);
	void					checkIfRegistered(Message& msg, std::string& resp);
};


#endif //SERVER_HPP
