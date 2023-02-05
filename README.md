# Ft_Irc
Simple Irc server (in progress)


## To-do:
- ### Kevin:

- ### Doush:
  - PRIVMSG
  - QUIT
  - USERS
     
- ### On hold:
  - LIST
  - MODE
  - Remove users from channel map efter disconnecting

- ### Done:
  - WHO (new)
  - PART
  - JOIN
  - TOPIC

## Notes:
- need to send a list of users after a user joins the channel (I think it's the same way the USERS command does it, so implementing that should fix it ? not sure), that's how the client knows who is on the channel, the WHO command is for information about those user (I think so, could be BS)
- Maybe we should consider putting some of the bigger commands in seperate files because they are getting long 
- Sorry if I broke something, did't have time to thoroughly test stuff
