#include <iostream>
#include <string>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// Non-blocking I/O
#include <memory>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>

int main()
{
  // Create a socket
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1)
  {
    return 1;
  }

  // Create a hint structure for the server we're connecting with
  int port = 54000;
  std::string ipAddress = "127.0.0.1";

  sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons(port);
  inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

  // Connect to the server on the socket
  int connectRes = connect(sock, (sockaddr *)&hint, sizeof(hint));
  if (connectRes == -1)
  {
    return 1;
  }

  char buf[4096];
  std::string userInput;

  memset(buf, 0, 4096);
  std::cout << "connected" << std::endl;
  int bytesReceived = recv(sock, buf, 4096, 0);
  std::cout << "connected-recv" << std::endl;
  if (bytesReceived == -1)
  {
    std::cout << "there was an error getting response from server\r\n";
    return 1;
  }
  else
  {
    std::cout << "Server> " << std::string(buf, bytesReceived);
  }

  // Reading user input
  std::mutex mutex;
  std::condition_variable cv;
  std::deque<std::string> lines;

  //    Enter lines of text
  std::thread io{[&]
                 {
                   std::string tmp;
                   while (true)
                   {
                     std::getline(std::cin, tmp);
                     std::lock_guard<std::mutex> lock{mutex};
                     lines.push_back(std::move(tmp));
                     cv.notify_one();
                   }
                 }};

  // Reading incoming messages
  std::mutex incMsgsMutex;
  std::condition_variable incMsgsCv;
  std::deque<std::string> incMsgs;

  //    Wait for response
  std::thread incMsgsIo{[&]
                        {
                          std::string tmp;
                          while (true)
                          {
                            memset(buf, 0, 4096);
                            int bytesReceived = recv(sock, buf, 4096, 0);
                            std::lock_guard<std::mutex> incMsgsLock{incMsgsMutex};
                            if (bytesReceived == -1)
                            {
                              tmp = std::string("not getting response from server\r\n");
                            }

                            tmp = std::string(buf, bytesReceived);
                            incMsgs.push_back(std::move(tmp));
                            incMsgsCv.notify_one();
                          }
                        }};

  // While loop:
  std::deque<std::string> toProcess;
  std::deque<std::string> incMsgsToProcess;
  while (true)
  {
    {
      std::unique_lock<std::mutex> lock{mutex};
      if (cv.wait_for(lock, std::chrono::seconds(0), [&]
                      { return !lines.empty(); }))
      {
        std::swap(lines, toProcess);
      }
    }

    {
      std::unique_lock<std::mutex> incMsgsLock{incMsgsMutex};
      if (incMsgsCv.wait_for(incMsgsLock, std::chrono::seconds(0), [&]
                             { return !incMsgs.empty(); }))
      {
        std::swap(incMsgs, incMsgsToProcess);
      }
    }

    //    Send to server
    if (!toProcess.empty())
    {
      for (auto &&line : toProcess)
      {
        int sendRes = send(sock, line.c_str(), line.size() + 1, 0);
        if (sendRes == -1)
        {
          std::cout << "could not send to server\r\n";
          continue;
        }
      }

      toProcess.clear();
    }

    //    Display response
    if (!incMsgsToProcess.empty())
    {
      for (auto &&msg : incMsgsToProcess)
      {
        std::cout << std::string(msg) << "\r\n";
      }

      incMsgsToProcess.clear();
    }
  }

  // Close the socket
  close(sock);

  return 0;
}

// while (true)
// {
//   //    Enter lines of text
//   std::cout << "> ";
//   getline(std::cin, userInput);

//   //    Send to server
//   int sendRes = send(sock, userInput.c_str(), userInput.size() + 1, 0);
//   if (sendRes == -1)
//   {
//     std::cout << "could not send to server\r\n";
//     continue;
//   }

//   //    Wait for response
//   memset(buf, 0, 4096);
//   int bytesReceived = recv(sock, buf, 4096, 0);
//   if (bytesReceived == -1)
//   {
//     std::cout << "there was an error getting response from server\r\n";
//   }
//   else
//   {
//     std::cout << "Server> " << std::string(buf, bytesReceived) << "\r\n";
//   }

//   //    Display response
// }
