First let's list the functions, then we'll talk about what they do.
# Functions:
```cpp
// This lambda acts each time a message is received. ClientID is who sent the message and msg is the message itself.
onMessage([](int ClientID, std::string msg){
  //logic here
});

// This is how you start your server, pick the port.
runServer(int port);

// This function connects to a server, the ip and port has to match the server itself.
// It returns a file descriptor.
runClient(std::string ip, int port);

// This send a message to the server/client.
// If a message is being sent from server to client, clientID returned by onMessage is put in variable int id.
// If a message is being sent from client to server, 1 is put in variable int id.
sendMsg(std::string msg, int id);

//This is returns the received message as a string
//When receiving from server to client, the previous file descriptor returned by runClient is put into variable "int id"
//Receiveing from client to server, you can just use onMessage, thats what it does.
recvMsg(int id)
```
## Note
- If you found any bugs, please let me know. Open a discussion in "Issues" or send me a private email and I'll happily fix it!
**Made by Mehdi B.R (Nullora @ Novus)**

