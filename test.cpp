#include "Channel.hpp"
#include "User.hpp"

#include <iostream>

int main(void)
{
    Channel chan = Channel("pokemon");
    User    kewin = User(0);
    User    doush = User(1);

    chan.addUser(kewin, OPERATOR);
    chan.addUser(doush, OPERATOR);
    // chan.findOperator(kewin);
    // chan.removeOperator(kewin);
    chan.setPrivilege(kewin, BANNED);
    std::cout << chan.findUser(kewin) << std::endl;
    return (0);
}