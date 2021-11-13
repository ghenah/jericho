#include <string>
#include <iostream>

#include "sockserv/tcp-listener.h"

TCPListener::TCPListener(std::string ipAddress, int port, MessageReceivedHandler MsgRecvHandler, ClientConnectedHandler ClntConnHandler, ClientDisconnectedHandler ClntDisconnHandler)
    : m_IPAddress(ipAddress), m_port(port), MessageReceived(MsgRecvHandler), ClientConnected(ClntConnHandler), ClientDisconnected(ClntDisconnHandler), ListeningSocket(0)
{
}

TCPListener::~TCPListener()
{
  Cleanup();
}

// Send a message to the specified client
void TCPListener::Send(int fd, std::string msg)
{
  if (!FD_ISSET(fd, &MainFDSet) || fd == ListeningSocket)
    return;

  send(fd, msg.c_str(), msg.size() + 1, 0);
}

// Initialize socket fd
bool TCPListener::Init()
{
  int ok = CreateSocket();
  if (ok == -1)
  {
    return false;
  }

  FD_ZERO(&MainFDSet);
  FD_SET(ListeningSocket, &MainFDSet);

  return true;
}

// The main processing looop
void TCPListener::Run()
{
  fd_set copyFDSet;
  int maxFD = ListeningSocket;

  while (true)
  {
    memcpy(&copyFDSet, &MainFDSet, sizeof(MainFDSet));

    int rc = select(maxFD + 1, &copyFDSet, nullptr, nullptr, nullptr);
    if (rc == -1)
      return;

    for (int req_fd = 0; req_fd <= maxFD; req_fd++)
    {
      if (!FD_ISSET(req_fd, &copyFDSet))
        continue;

      if (req_fd == ListeningSocket)
      {
        // Accept a new connection
        int clientFD = accept(ListeningSocket, nullptr, nullptr);
        if (clientFD == -1)
        {
          std::cout << "couldn't accept a new client socket" << std::endl;
        }
        // Add the new connection to the list of connected clients
        FD_SET(clientFD, &MainFDSet);
        if (clientFD > maxFD)
          maxFD = clientFD;

        if (ClientConnected != NULL)
          ClientConnected(this, clientFD);
      }
      else
      {
        // Accept a new message
        char buf[MAX_BUFFER_SIZE];
        memset(buf, 0, MAX_BUFFER_SIZE);

        int bytesIn = recv(req_fd, buf, MAX_BUFFER_SIZE, 0);
        if (bytesIn <= 0)
        {
          // Drop the client
          close(req_fd);
          FD_CLR(req_fd, &MainFDSet);

          if (ClientDisconnected != NULL)
            ClientDisconnected(this, req_fd);
        }
        else
        {
          if (MessageReceived != NULL)
            MessageReceived(this, req_fd, std::string(buf, 0, bytesIn));
        }
      }
    }
  }

  close(ListeningSocket);
}

void TCPListener::Cleanup()
{
}

int TCPListener::CreateSocket()
{
  ListeningSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (ListeningSocket != -1)
  {
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(m_port);
    inet_pton(AF_INET, m_IPAddress.c_str(), &hint.sin_addr);

    int bindOk = bind(ListeningSocket, (sockaddr *)&hint, sizeof(hint));
    if (bindOk != -1)
    {
      int listenOk = listen(ListeningSocket, SOMAXCONN);
      if (listenOk == -1)
      {
        return -1;
      }
    }
    else
    {
      return -1;
    }
  }

  return 0;
}
