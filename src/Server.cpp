#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "XPLMUtilities.h"

#include "Handler.cpp"

// https://bruinsslot.jp/post/simple-http-server-in-c/

int sockfd;
struct sockaddr_in host_addr;
int host_addrlen;

#define PORT 8080
#define BUFFER_SIZE 1024 * 1000 // 1mb

char buffer[BUFFER_SIZE];

std::string common_headers = "Server: fly-with-http\r\n"
                             "Content-type: application/json\r\n";

int has_accept()
{
  // Definitions
  fd_set stReadFDS;
  fd_set stExceptFDS;
  struct timeval timeout;

  // Setup for Select
  FD_ZERO(&stReadFDS);
  FD_SET(sockfd, &stReadFDS);

  // Set timeout period for select to 0 to poll the socket
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  int status = select(sockfd + 1, &stReadFDS, NULL, NULL, &timeout);

  return status;
}

void parseHead(std::string *head, Request *request)
{
  std::size_t path_offset;
  std::size_t version_offset;

  path_offset = head->find(" ");
  request->method = head->substr(0, path_offset);
  path_offset += 1;

  version_offset = head->find(" ", path_offset);
  request->path = head->substr(path_offset, version_offset - path_offset);
  version_offset += 1;

  request->version = head->substr(version_offset);
}

float serverLoop()
{
  using namespace std;

  int status = has_accept();

  // No one connection in queue
  if (status <= 0)
  {
    return -1;
  }

  XPLMDebugString("Connection\n");

  // Accept incoming connections
  int connection_fd = accept(sockfd, NULL, NULL);

  XPLMDebugString("connection accepted\n");

  if (connection_fd < 0)
  {
    XPLMDebugString("ERR: server accept\n");
    return -1;
  }

  // int buffer_length = read(connection_fd, buffer, BUFFER_SIZE);
  int buffer_length = recv(connection_fd, buffer, BUFFER_SIZE, MSG_DONTWAIT);

  if (buffer_length < 0)
  {
    XPLMDebugString("ERR: server read\n");
    close(connection_fd);
    return -1;
  }

  XPLMDebugString("received\n");

  struct Request request;

  request.raw = string(buffer, buffer_length);

  string head;
  size_t headers_offset = request.raw.find("\r\n");

  if (headers_offset == string::npos)
  {
    XPLMDebugString("ERR: server start line\n");
    close(connection_fd);
    return -1;
  }

  head = request.raw.substr(0, headers_offset);
  headers_offset += 2;

  parseHead(&head, &request);

  if (request.version.compare("HTTP/1.1") != 0)
  {
    XPLMDebugString("ERR: server http version\n");
    close(connection_fd);
    return -1;
  }

  size_t body_offset = request.raw.find("\r\n\r\n");

  request.headers = request.raw.substr(headers_offset, body_offset - headers_offset);

  if (body_offset != string::npos)
  {
    body_offset += 4;
    request.body = request.raw.substr(body_offset);
  }

  XPLMDebugString("request parsed\n");

  struct Response response;

  try
  {
    Route(&request, &response);
  }
  catch (...)
  {
    XPLMDebugString("Route exception\n");
  }

  XPLMDebugString("sending response\n");

  // Write to the socket
  std::string response_head;

  buffer_length = sprintf(buffer, "HTTP/1.0 %i %s\r\n", response.code, response.status.c_str());
  response_head.append(buffer, buffer_length);
  response_head.append(common_headers);
  buffer_length = sprintf(buffer, "Content-Length: %lu\r\n", response.body.length());
  response_head.append(buffer, buffer_length);
  response_head.append(response.headers);
  response_head.append("\r\n");

  status = write(connection_fd, response_head.c_str(), response_head.length());

  if (status < 0)
  {
    XPLMDebugString("ERR: server write head\n");
    close(connection_fd);
    return -1;
  }

  status = write(connection_fd, response.body.c_str(), response.body.length());

  if (status < 0)
  {
    XPLMDebugString("ERR: server write body\n");
    close(connection_fd);
    return -1;
  }

  close(connection_fd);

  XPLMDebugString("connection closed normally\n");
  return -1;
}

int ServerListen()
{
  // Create a socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd == -1)
  {
    XPLMDebugString("ERR: server socket\n");
    return 0;
  }

  XPLMDebugString("socket created successfully\n");

  // Create the address to bind the socket to
  host_addrlen = sizeof(host_addr);

  host_addr.sin_family = AF_INET;
  host_addr.sin_port = htons(PORT);
  host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // Bind the socket to the address
  if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0)
  {
    XPLMDebugString("ERR: server bind\n");
    return -1;
  }

  XPLMDebugString("socket successfully bound to address\n");

  // Listen for incoming connections
  if (listen(sockfd, SOMAXCONN) != 0)
  {
    XPLMDebugString("ERR: server listen\n");
    return -1;
  }

  XPLMDebugString("server listening for connections\n");

  return 0;
}

int ServerStop()
{
  int status = close(sockfd);

  return status;
}