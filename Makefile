NAME := ircserv
CPP := c++
CPPFLAGS := -std=c++98 -Wall -Wextra -Werror -pthread
# CPPFLAGS := -std=c++98
IFLAGS := -I .

VPATH := commands

SRC :=	Message.cpp Server.cpp User.cpp main.cpp simpleCommands.cpp who.cpp channelCommands.cpp \
		privmsg.cpp login.cpp mode.cpp Channel.cpp helperFunctions.cpp ChannelBot.cpp
OBJ := $(addprefix _obj/,$(notdir $(SRC:.cpp=.o)))

DEPS := Message.hpp Server.hpp User.hpp Channel.hpp

all: $(NAME)

$(NAME): $(OBJ) $(DEPS)
	@$(CPP) $(CPPFLAGS) $(OBJ) -o $(NAME)
	@printf "\033[32;1mCompilation Completed\033[0m\n"
	@printf "\033[32;3mProgram name: $(NAME)\033[0m\n"
	@printf "\033[32;3mRun: ./$(NAME) <port> <password>\033[0m\n"

_obj:
	mkdir _obj

_obj/%.o: %.cpp $(DEPS) | _obj
	@$(CPP) $(CPPFLAGS) $(IFLAGS) -c $< -o $@

clean:
	@rm -rf _obj
	@clear
	@printf "\033[31mObject files cleaned\033[0m\n"

fclean: clean
	@rm -rf $(NAME)
	@printf "\033[31mExecutable cleaned\033[0m\n"

re: fclean all
