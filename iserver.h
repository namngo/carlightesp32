#ifndef ISERVER_H
#define ISERVER_H

#include <functional>
#include <map>
#include <string>

#include "Arduino.h"

namespace carlight {

// Interface of a server, which can be WifiServer, SerialServer, BluetoothServer
class IServer {
 public:
  typedef std::map<String, String> ParamMap;
  typedef std::function<String(const String& url, const String& body_json)>
      TJsonPostHandler;
  typedef std::function<String(const String& url, const ParamMap& params)>
      TJsonGettHandler;

  virtual void Begin();

  virtual void onGetJson(const String& url, TJsonGettHandler handler);

  virtual void onPostJson(const String& url, TJsonPostHandler handler);

  virtual void Loop();
};
}  // namespace carlight

#endif
