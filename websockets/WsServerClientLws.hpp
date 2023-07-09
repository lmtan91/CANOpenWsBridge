
#ifndef ARISTA_WEBSOCKET_SERVER_CLIENT_LWS_H
#define ARISTA_WEBSOCKET_SERVER_CLIENT_LWS_H

#include "Core.hpp"
#include "Subject.hpp"
#include "WsServerClient.hpp"

#include <chrono>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace arista {

struct WsServerClientLwsImpl;

class WsServerClientLws : public WsServerClient {
  public:
    explicit WsServerClientLws(WsServerClientLwsImpl* d);
    WsServerClientLws(WsServerClientLws&) = delete;
    ~WsServerClientLws();

    void send(const LwsPacket& message) override;
    void sendHttpResult(const HttpRequestResult& result) override;

    std::unique_ptr<arista::Subscription> onDisconnect(std::function<void(void)> callback)
    {
        auto observer = std::make_shared<arista::FuncObserver<void>>(callback);
        return m_onDisconnectSubject.subscribe(observer);
    }

    // TODO private??
    void registerMessageHandler(std::function<void(const LwsPacket&)> callback);
    void handleMessage(const LwsPacket& message);
    void handleDisconnect();

  private:
    WsServerClientLwsImpl* d { nullptr };

    int m_messageHandlerId {};
    std::map<int, std::function<void(const LwsPacket&)>> m_messageHandlers;

    arista::Subject<void> m_onDisconnectSubject;
};

} // namespace arista

#endif
