#include <iostream>
#include <sys/socket.h>
#include <netdb.h>
#include "Server.hpp"

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cout << "Please provide the port and password" << std::endl;
		return 1;
	}
	try {
		Server server(argv);
		std::cout << "port: " << server.getPort() << std::endl;
		std::cout << "pass: " << server.getPassword() << std::endl;
		server.start();
	} catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	return 0;
}
