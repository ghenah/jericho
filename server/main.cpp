#include <iostream>
#include <string>

#include "sockserv/tcp-listener.h"

int listIndex = 0;
int listSize = 10;
int list[10];

void MessageReceived(TCPListener *listener, int client, std::string msg);
void ClientConnected(TCPListener *listener, int client);
void ClientDisconnected(TCPListener *listener, int client);

int connectToContentModerator(int port);

int main()
{
  TCPListener server("127.0.0.1", 54000, MessageReceived, ClientConnected, ClientDisconnected);

  if (server.Init())
  {
    server.Run();
  }
  else
  {
    std::cout << "error: failed to initialize" << std::endl;

    return -1;
  }

  return 0;
}

void MessageReceived(TCPListener *listener, int authorFD, std::string msg)
{
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