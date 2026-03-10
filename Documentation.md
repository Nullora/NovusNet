*First let's list the functions, then we'll talk about what they do.*
# Functions:
```cpp
// This callback is called each time a message is received. ClientID is who sent the message and msg is the message itself.
// This is server side only
onMessage([](int ClientID, std::string msg){
  //EXAMPLE:
  //Everytime the server receives a message, the function prints the sender and the message itself.
  std::cout<<ClientID<<' '<<msg<<endl;
});

// This is how you start your server, pick the port amd the password that each client needs to connect.
runServer(int port, std::string password);

// This function connects to a server, the ip, port, and password has to match the server's.
// It returns a file descriptor.
runClient(std::string ip, int port, std::string password);

// This send a message to the server/client.
// If a message is being sent from server to client, clientID returned by onMessage is put in variable int id.
// If a message is being sent from client to server, a FileDescriptor returned by runClient is put in variable int id.
sendMsg(std::string msg, int id);

//This is returns the received message as a string
//When receiving from server to client, the previous file descriptor returned by runClient is put into variable "int id"
//Receiveing from client to server, you use onMessage, thats what it does.
recvMsg(int id)
```
## Note
- If you found any bugs, please let me know. Open a discussion in "Issues" or send me a private email and I'll happily fix it!

