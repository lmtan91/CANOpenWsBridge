
#ifndef ARISTA_WEBSOCKET_LWS_H
#define ARISTA_WEBSOCKET_LWS_H

#include "Core.hpp"
#include "WorkerThread.hpp"
#include "WsServerClientLws.hpp"

#include "HttpRequest.hpp"

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace arista {

struct WsServerLwsPriv;

class WsServerLws {
  public:
    WsServerLws();
    ~WsServerLws();

    void init(int port);
    void stop();
    void join();

    void wakeup();

    void handleWebSocket(const std::string& url, const std::function<void(WsServerClient&)>& callback);
    void handle(arista::HttpMethod method, const std::string& url, const std::function<void(WsServerClient&, const HttpRequest&)>& callback);

  private:
    bool work();

    WsServerLwsPriv* d;
    WorkerThread m_workerThread;
};

} // namespace arista

#endif
