# Ft_Irc
Simple Irc server (in progress)


## To-do:
- ### Kevin:
  - MODE
  - JOIN (finish)
  - PART (needs to send a message to other users when someone leaves, use sendToChannel)

- ### Doush:
  - QUIT
  - USERS
     
- ### On hold:
  - LIST
  - INVITE
  - KICK
  - Remove users from channel map efter disconnecting --can probably add this into QUIT

- ### Done:
  - PRIVMSG
  - WHO (new)
  - PART
  - JOIN
  - TOPIC
  - Sending the channel users and the channel topic when someone joins the channel

## Notes:
- Maybe we should consider putting some of the bigger commands in seperate files because they are getting long
- Sorry if I broke something, did't have time to thoroughly test stuff
## MODE, JOIN:
- Still need to do mode changes for users
- Need to enforce -i (invite only) flag for JOIN, probably more flags also have to be enforced still
- I think I might have to send the message that MODE has been changed to all members like Doush did for JOIN, WHO