#include <iostream>
#include <string>
#include <errno.h>

#include "sockserv/tcp-listener.h"

int listIndex = 0;
int listSize = 10;
int list[10];

void MessageReceived(TCPListener *listener, int client, std::string msg);
void ClientConnected(TCPListener *listener, int client);
void ClientDisconnected(TCPListener *listener, int client);

int cmSocket = 0;
int connectToContentModerator(std::string addr, int port);
int contentModeratorFilterMessage(std::string &msg);

int main()
{
  cmSocket = connectToContentModerator("127.0.0.1", 54010);
  if (cmSocket == -1)
  {
    std::cout << "error: failed to connect to content moderator" << std::endl;

    return -1;
  }

  TCPListener server("127.0.0.1", 54000, MessageReceived, ClientConnected, ClientDisconnected);

  if (server.Init())
  {
    server.Run();
  }
  else
  {
    std::cout << "error: failed to initialize" << std::endl;
    close(cmSocket);

    return -1;
  }

  close(cmSocket);
  return 0;
}

void MessageReceived(TCPListener *listener, int authorFD, std::string msg)
{
  if (contentModeratorFilterMessage(msg) == -1)
  {
    std::cerr << "could not filter the message; aborting broadcasting" << std::endl;
    return;
  }

  for (int i = 0; i < listSize; i++)
  {
    if (list[i] > 0)
    {
      listener->Send(list[i], msg);
    }
  }
}

void ClientConnected(TCPListener *listener, int fd)
{
  if (0 <= listIndex && listIndex <= listSize)
  {
    list[listIndex++] = fd;

    std::cout << "client connected: " << fd << std::endl;
  }
  else
  {
    std::cerr << "couldn't add new client: " << fd << std::endl;
  }
}

void ClientDisconnected(TCPListener *listener, int fd)
{
  if (0 <= listIndex && listIndex <= listSize)
  {
    list[listIndex++] = 0;

    std::cout << "client disconnected: " << fd << std::endl;
  }
  else
  {
    std::cerr << "couldn't remove client: " << fd << std::endl;
  }
}

int connectToContentModerator(std::string addr, int port)
{
  // Create a socket
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1)
  {
    return -1;
  }

  // Create a hint structure for the server we're connecting with
  sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(port);
  inet_pton(AF_INET, addr.c_str(), &hint.sin_addr);

  // Connect to the server on the socket
  int connectRes = connect(sock, (sockaddr *)&hint, sizeof(hint));
  if (connectRes == -1)
  {
    return -1;
  }

  return sock;
}

int contentModeratorFilterMessage(std::string &msg)
{

  int sendRes = send(cmSocket, msg.c_str(), msg.size() + 1, 0);
  if (sendRes == -1)
  {
    std::cerr << "failure when sending to cm: errno" << errno << std::endl;
    return -1;
  }

  char buf[MAX_BUFFER_SIZE];

  memset(buf, 0, MAX_BUFFER_SIZE);
  int bytesReceived = recv(cmSocket, buf, MAX_BUFFER_SIZE, 0);
  if (bytesReceived == -1)
  {
    std::cerr << "failure when reading from cm" << std::endl;
    return -1;
  }

  msg = std::string(buf, bytesReceived);
  return 0;
}
//    Wait for response
// std::thread incMsgsIo{[&]
//                       {
//                         std::string tmp;
//                         while (true)
//                         {
//                           memset(buf, 0, MAX_BUFFER_SIZE);
//                           int bytesReceived = recv(sock, buf, MAX_BUFFER_SIZE, 0);
//                           std::lock_guard<std::mutex> incMsgsLock{incMsgsMutex};
//                           if (bytesReceived == -1)
//                           {
//                             tmp = std::string("not getting response from server\r\n");
//                           }

//                           tmp = std::string(buf, bytesReceived);
//                           incMsgs.push_back(std::move(tmp));
//                           incMsgsCv.notify_one();
//                         }
//                       }};

// // While loop:
// std::deque<std::string> toProcess;
// std::deque<std::string> incMsgsToProcess;
// while (true)
// {
//   {
//     std::unique_lock<std::mutex> lock{mutex};
//     if (cv.wait_for(lock, std::chrono::seconds(0), [&]
//                     { return !lines.empty(); }))
//     {
//       std::swap(lines, toProcess);
//     }
//   }

//   {
//     std::unique_lock<std::mutex> incMsgsLock{incMsgsMutex};
//     if (incMsgsCv.wait_for(incMsgsLock, std::chrono::seconds(0), [&]
//                            { return !incMsgs.empty(); }))
//     {
//       std::swap(incMsgs, incMsgsToProcess);
//     }
//   }

//   //    Send to server
//   if (!toProcess.empty())
//   {
//     for (auto &&line : toProcess)
//     {
//       int sendRes = send(sock, line.c_str(), line.size() + 1, 0);
//       if (sendRes == -1)
//       {
//         std::cout << "could not send to server\r\n";
//         continue;
//       }
//     }

//     toProcess.clear();
//   }

//   //    Display response
//   if (!incMsgsToProcess.empty())
//   {
//     for (auto &&msg : incMsgsToProcess)
//     {
//       std::cout << std::string(msg) << "\r\n";
//     }

//     incMsgsToProcess.clear();
//   }
// }
// }
