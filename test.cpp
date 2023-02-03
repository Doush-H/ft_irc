#include "Channel.hpp"
#include "User.hpp"

#include <iostream>

int main(void)
{
    Channel chan = Channel("pokemon", NONE);
    User    kewin = User(0);
    User    doush = User(1);

    chan.addUser(kewin, OPERATOR);
    chan.addUser(doush, OPERATOR);
    // chan.findOperator(kewin);
    // chan.removeOperator(kewin);
    chan.setPrivilege(kewin, BANNED);
	std::cout << chan.checkModes(PRIV) << std::endl;
	chan.setModes(PRIV | SECRET);
	std::cout << chan.checkModes(PRIV | SECRET) << std::endl;
	// std::cout << chan.checkModes(MODERATED) << std::endl;

	std::cout << "-------------------------" << std::endl;

	chan.removeModes(PRIV);
	std::cout << chan.checkModes(PRIV) << std::endl;
	std::cout << chan.checkModes(SECRET) << std::endl;
	std::cout << chan.checkModes(MODERATED) << std::endl;
    return (0);
}