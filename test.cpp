#include "Channel.hpp"
#include "User.hpp"

int main(void)
{
    Channel chan = Channel("pokemon");
    User    kewin = User(0);

    chan.addOperator(kewin);
    chan.addBanned(kewin);
    // chan.findOperator(kewin);
    // chan.removeOperator(kewin);
    chan.findOperator(kewin);
    return (0);
}