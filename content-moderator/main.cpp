#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "cs106b/tokenscanner.h"
#include "sockserv/tcp-listener.h"

void MessageReceived(TCPListener *listener, int client, std::string msg);
void ClientConnected(TCPListener *listener, int client);
void ClientDisconnected(TCPListener *listener, int client);

void filterOutCurseWords(std::string &text);

int main()
{
  TCPListener server("127.0.0.1", 54010, MessageReceived, ClientConnected, ClientDisconnected);

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

void filterOutCurseWords(std::string &text)
{
  TokenScanner scanner;
  scanner.setInput(text);
  std::string filteredText;

  while (scanner.hasMoreTokens())
  {
    std::string token = scanner.nextToken();

    if (token == "fuck")
      token = "****";

    filteredText.append(token);
  }

  text = filteredText;

  return;
}

void MessageReceived(TCPListener *listener, int authorFD, std::string msg)
{
  filterOutCurseWords(msg);
  listener->Send(authorFD, msg);
}

void ClientConnected(TCPListener *listener, int fd)
{
  if (0 <= fd)
  {
    std::cout << "client connected: " << fd << std::endl;
  }
  else
  {
    std::cerr << "couldn't add new client: " << fd << std::endl;
  }
}

void ClientDisconnected(TCPListener *listener, int fd)
{
  if (0 <= fd)
  {
    std::cout << "client disconnected: " << fd << std::endl;
  }
}