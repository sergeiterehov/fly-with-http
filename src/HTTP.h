#include <string.h>

#ifndef _HTTP_H
#define _HTTP_H

struct Request
{
  std::string raw;

  std::string method;
  std::string path;
  std::string version;
  std::string headers;
  std::string body;
};

struct Response
{
  int code = 200;
  std::string status = "OK";
  std::string headers;
  std::string body;
};

#endif
