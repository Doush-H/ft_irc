# Ft_Irc
Simple Irc server (in progress)


## To-do:
- ### kewin:

- ### Doush:
     
- ### On hold:
  - KICK

- ### Done:
  - INVITE
  - LIST
  - USERS (not doing it because it's apparently disabled in most of the irc servers, it's similar to WHO command too)
  - QUIT
  - MODE
  - PRIVMSG
  - WHO (new)
  - PART
  - JOIN
  - TOPIC
  - Sending the channel users and the channel topic when someone joins the channel

## Notes:
- Sorry if I broke something, did't have time to thoroughly test stuff
- !!! IMPORTANT !!! the syntax for the numeric replies is: "SERVER_PREFIX *numeric reply number* *sender_nick* *other params for the numeric reply which are specified in the documents*". All the numeric replies should have the sender_nick as the first parameter.
- Still need to do mode changes for users
- Need to enforce -i (invite only) flag for JOIN, probably more flags also have to be enforced still
- I think I might have to send the message that MODE has been changed to all members like Doush did for JOIN, WHO
