#ifndef ESPCONTROLLER_H
#define ESPCONTROLLER_H

#include <map>
#include <vector>

#include "Arduino.h"
#include "iserver.h"

namespace carlight {

// Controls all of the IServer object
class Controller : public IServer {
 public:
  Controller() : servers_() {}

  Controller(std::vector<std::unique_ptr<IServer>> servers)
      : servers_(std::move(servers)) {}

  void addServer(std::unique_ptr<IServer> server) {
    servers_.push_back(std::move(server));
  }

  void Begin() {
    for (const auto &srv : servers_) {
      if (srv.get() != nullptr) {
        srv.get()->Begin();
      }
    }
  }

  void onGetJson(const String &url, TJsonHandler handler) {
    for (const auto &srv : servers_) {
      if (srv.get() != nullptr) {
        srv.get()->onGetJson(url, handler);
      }
    }
  }

  void onPostJson(const String &url, TJsonHandler handler) {
    for (const auto &srv : servers_) {
      if (srv.get() != nullptr) {
        srv.get()->onPostJson(url, handler);
      }
    }
  }

  void Loop() {
    for (const auto &srv : servers_) {
      if (srv.get() != nullptr) {
        srv.get()->Loop();
      }
    }
  }

 private:
  std::vector<std::unique_ptr<IServer>> servers_;
};

}  // namespace carlight

#endif
