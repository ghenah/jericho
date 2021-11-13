#pragma once

#include <string>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX_BUFFER_SIZE (49152)

// Forward declaration of class
class TCPListener;

// Callback to data received
typedef void (*MessageReceivedHandler)(TCPListener *listener, int socketID, std::string msg);
// Callback to client connected
typedef void (*ClientConnectedHandler)(TCPListener *listener, int socketID);
// Callback to client disconnected
typedef void (*ClientDisconnectedHandler)(TCPListener *listener, int socketID);

class TCPListener
{
public:
  TCPListener(std::string ipAddress, int port, MessageReceivedHandler MsgRecvHandler, ClientConnectedHandler ClntConnHandler, ClientDisconnectedHandler ClntDisconnHandler);

  ~TCPListener();

  // Send a message to the specified client
  void Send(int clientSocket, std::string msg);

  // Initialize socket fd
  bool Init();

  // The main processing looop
  void Run();

  void Cleanup();

  // Receive loop
  //   Send back message
  // Cleanup

private:
  // Create a socket
  int CreateSocket();

  // Wait for a connection
  int WaitForConnection(int listening);

  std::string m_IPAddress;
  int m_port;
  MessageReceivedHandler MessageReceived;
  ClientConnectedHandler ClientConnected;
  ClientDisconnectedHandler ClientDisconnected;
  int ListeningSocket;
  fd_set MainFDSet;
};