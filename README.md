# Ft_Irc
Simple Irc server (in progress)


## To-do:
- ### kewin: join or part multiple
  - JOIN (finish)
  - PART (needs to send a message to other users when someone leaves, use sendToChannel)

- ### Doush:
     
- ### On hold:
  - INVITE
  - KICK

- ### Done:
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
## MODE, JOIN:
- Still need to do mode changes for users
- Need towwq enforce -i (invite only) flag for JOIN, probably more flags also have to be enforced still
- I think I might have to send the message that MODE has been changed to all members like Doush did for JOIN, WHO