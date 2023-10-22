#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "nlohmann/json.hpp"

#include "HTTP.h"

bool isEqual(std::string a, char *b)
{
  return a.compare(b) == 0;
}

bool startsWith(std::string a, char *b)
{
  return a.substr(0, strlen(b)).compare(b) == 0;
}

bool subRoute(char *prefix, Request *req)
{
  if (startsWith(req->path, prefix))
  {
    req->path.erase(0, strlen(prefix));

    return true;
  }

  return false;
}

void GetIndex(Request *req, Response *res)
{
  XPLMDataRef ref = XPLMFindDataRef("sim/cockpit/electrical/beacon_lights_on");
  int state = XPLMGetDatai(ref);

  nlohmann::json response_json = nlohmann::json::object();

  response_json["beacon_lights_on"] = state;

  res->body = response_json.dump();
}

void GetBegin(Request *req, Response *res)
{
  XPLMCommandRef ref = XPLMFindCommand(req->path.c_str());

  if (!ref)
  {
    res->code = 404;
    res->status = "Not Found";
    res->body = nlohmann::json::object({{"command",
                                         req->path}})
                    .dump();
    return;
  }

  XPLMCommandBegin(ref);
}

void GetEnd(Request *req, Response *res)
{
  XPLMCommandRef ref = XPLMFindCommand(req->path.c_str());

  if (!ref)
  {
    res->code = 404;
    res->status = "Not Found";
    res->body = nlohmann::json::object({{"command",
                                         req->path}})
                    .dump();
    return;
  }

  XPLMCommandEnd(ref);
}

void GetOnce(Request *req, Response *res)
{
  XPLMCommandRef ref = XPLMFindCommand(req->path.c_str());

  if (!ref)
  {
    res->code = 404;
    res->status = "Not Found";
    res->body = nlohmann::json::object({{"command",
                                         req->path}})
                    .dump();
    return;
  }

  XPLMCommandOnce(ref);
}

void Route(Request *req, Response *res)
{
  if (isEqual(req->method, "GET"))
  {
    if (isEqual(req->path, "/"))
    {
      return GetIndex(req, res);
    }

    if (subRoute("/begin/", req))
    {
      return GetBegin(req, res);
    }

    if (subRoute("/end/", req))
    {
      return GetEnd(req, res);
    }

    if (subRoute("/once/", req))
    {
      return GetOnce(req, res);
    }
  }

  res->code = 404;
  res->status = "Not found";
  res->body = nlohmann::json::object({{"method", req->method}}).dump();
}
