#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main()
{
  // Create socket
  int listening = socket(AF_INET, SOCK_STREAM, 0);
  if (listening == -1)
  {
    std::cerr << "can't create a socket";
    return -1;
  }
  std::cout << "listening on fd: " << listening << std::endl;

  // Bind the socket to an IP / port
  sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(54000);
  inet_pton(AF_INET, "0.0.0.0", &hint.sin_addr);

  if (bind(listening, (sockaddr *)&hint, sizeof(hint)) == -1)
  {
    std::cerr << "can't bind to IP/port";
    return -2;
  }

  // Mark the socket for listening in
  if (listen(listening, SOMAXCONN) == -1)
  {
    std::cerr << "can't listen";
    return -3;
  }

  fd_set main_set;
  fd_set copy_set;

  FD_ZERO(&main_set);
  FD_SET(listening, &main_set);
  int max_fd = listening;

  while (true)
  {
    memcpy(&copy_set, &main_set, sizeof(main_set));

    int rc = select(max_fd + 1, &copy_set, nullptr, nullptr, nullptr);
    if (rc == -1)
    {
      std::cerr << "select failed: errno = " << errno << std::endl;

      return 1;
    }

    for (int req_fd = 0; req_fd <= max_fd; req_fd++)
    {
      if (!FD_ISSET(req_fd, &copy_set))
      {

        continue;
      }

      if (req_fd == listening)
      {
        // Accept a new connection
        int client_sock = accept(listening, nullptr, nullptr);
        if (client_sock == -1)
        {
          std::cout << "couldn't accept a new client socket" << std::endl;
        }
        // Add the new connection to the list of connected clients
        FD_SET(client_sock, &main_set);
        if (client_sock > max_fd)
          max_fd = client_sock;

        // Send a welcome message to the connected client
        std::string welcomeMsg = "You have connected to the server.\r\n";
        send(client_sock, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);

        // TODO: Broadcast we have a new connection
      }
      else
      {
        // Accept a new message
        char buf[4096];
        memset(buf, 0, 4096);

        int bytesIn = recv(req_fd, buf, 4096, 0);
        std::cout << "message received: " << std::string(buf) << std::endl;
        if (bytesIn <= 0)
        {
          // Drop the client
          close(req_fd);
          FD_CLR(req_fd, &main_set);
        }
        else
        {
          // Send message to other clients, and definitely NOT the listening socket
          for (int fd = 0; fd <= max_fd; fd++)
          {
            if (!FD_ISSET(fd, &main_set) || fd == listening)
              continue;

            send(fd, buf, bytesIn, 0);
          }
        }
      }
    }
  }

  // Close socket
  // close(clientSocket);

  return 0;
}

// // Accept a call
// sockaddr_in client;
// socklen_t clientSize = sizeof(client);
// char host[NI_MAXHOST];
// char svc[NI_MAXSERV];

// int clientSocket = accept(listening, (sockaddr *)&client, &clientSize);

// if (clientSocket == -1)
// {
//   std::cerr << "problem with client connecting";
//   return -4;
// }

// // Close the listening socket
// close(listening);

// memset(host, 0, NI_MAXHOST);
// memset(svc, 0, NI_MAXSERV);

// int result = getnameinfo((sockaddr *)&client, sizeof(client), host, NI_MAXHOST, svc, NI_MAXSERV, 0);

// if (result)
// {
//   std::cout << host << " connected on " << svc << std::endl;
// }
// else
// {
//   inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
//   std::cout << host << " connected on " << ntohs(client.sin_port) << std::endl;
// }

// // While receiving display message, echo message
// char buf[4096];
// while (true)
// {
//   // Clear the buffer
//   memset(buf, 0, 4096);

//   // Wait for a message
//   int bytesRecv = recv(clientSocket, buf, 4096, 0);
//   if (bytesRecv == -1)
//   {
//     std::cerr << "there was a connection issue" << std::endl;
//     break;
//   }

//   if (bytesRecv == 0)
//   {
//     std::cout << "the client disconnected" << std::endl;
//     break;
//   }

//   // Display message
//   std::cout << "Received: " << std::string(buf, 0, bytesRecv) << std::endl;

//   // Resend message
//   send(clientSocket, buf, bytesRecv + 1, 0);
// }