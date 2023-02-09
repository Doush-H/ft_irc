# Ft_Irc
Simple Irc server (in progress)


## To-do:
- ### kewin:

- ### Doush:
     
- ### On hold:

- ### Stuff we can still do:
  - OPER
  - NOTICE
  - SENDFILE
  - GETFILE
  - Helper bot / bouncer bot

- ### Done:
  - KICK
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
- !!! IMPORTANT !!! the syntax for the numeric replies is: "SERVER_PREFIX *numeric reply number* *sender_nick* *other params for the numeric reply which are specified in the documents*". All the numeric replies should have the sender_nick as the first parameter.
- Need to enforce -i (invite only) flag for JOIN, probably more flags also have to be enforced still
