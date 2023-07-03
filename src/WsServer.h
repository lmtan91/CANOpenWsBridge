#ifndef WSSERVER_H
#define WSSERVER_H

#include <vector>

#include "Util.hpp"
#include "WorkerThread.hpp"
#include "libwebsockets.h"

namespace arista {
class WsServer {
  public:
    WsServer();
    ~WsServer();

    void init(int port);
    void stop();
    void join();

    void wakeup();

    // void handleWebSocket(const std::string& url, const std::function<void(WebSocketServerClient&)>& callback);

    const lws_protocol_vhost_options pvo = { nullptr, nullptr, "websocket-server", "" };
    lws_http_mount mount {};
    lws_context_creation_info info {};
    lws_context* context { nullptr };

    std::vector<lws_protocols> protocols;

  private:
    bool work();

    WorkerThread m_workerThread;
};

}

#endif
