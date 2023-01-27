#include <iostream>
#include "Message.hpp"
#include <list>

// ------------------- A file for testing Message class ----------------------
int main() {
	std::string buf = "NICK BADCoder69 * *";
	Message msg = Message::parseBuf(buf);
	std::cout << msg << std::endl;
}