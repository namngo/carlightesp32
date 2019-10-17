#ifndef ESPCONTROLLER_H
#define ESPCONTROLLER_H

#include <unique_ptr.h>

#include <map>
#include <vector>

#include "Arduino.h"
#include "iserver.h"

namespace carlight {

// Controls all of the IServer object
class Controller : public IServer {
 public:
  Controller(const std::vector<std::unique_ptr<IServer>>& servers)
      : servers_(servers) {}

  void onGetJson(String url, TJsonGettHandler handler) {
    for (const auto& srv : servers_) {
      if (srv.get() != nullptr) {
        srv.get()->onGetJson(url, handler);
      }
    }
  }

  void onPostJson(String url, TJsonPostHandler handler) {
    for (const auto& srv : servers_) {
      if (srv.get() != nullptr) {
        srv.get()->onPostJson(url, handler);
      }
    }
  }

 private:
  std::vector<std::unique_ptr<IServer>> servers_;
};

}  // namespace carlight

#endif
