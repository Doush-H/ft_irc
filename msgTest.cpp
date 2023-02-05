#include <iostream>
// #include "Message.hpp"
// #include <list>

// ------------------- A file for testing Message class ----------------------
bool compare(const char* mask, const char* str) {
	if (mask[0] == '*') {
		do {
			if (compare(mask + 1, str)) {
				return true;
			}
		} while (*(str++));
		return false;
	} else if (mask[0] != str[0]) {
		return false;
	} else if (*str) {
		return compare(mask + 1, str + 1);
	}
	return true;
}

int main() {
	std::string buf = "doush";
	std::string mask = "*sh";
	std::cout << compare(mask.c_str(), buf.c_str()) << std::endl;
}
