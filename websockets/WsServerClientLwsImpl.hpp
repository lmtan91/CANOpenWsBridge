
#ifndef ARISTA_WEBSOCKET_SERVER_CLIENT_LWS_IMPL_H
#define ARISTA_WEBSOCKET_SERVER_CLIENT_LWS_IMPL_H

#include "Core.hpp"
#include "WsServerClient.hpp"
#include "IncomingHttpRequest.hpp"
#include "libwebsockets.h"

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <mutex>

namespace arista {

class WsServerClientLws;

// TODO can be removed?
struct VhdWsServer
{
    lws_context* context;
    lws_vhost* vhost;
};

class WsServerClientLwsImpl {
  public:
    WsServerClientLwsImpl(lws_context* serverContext, lws* wsi);
    ~WsServerClientLwsImpl();

    //// used by lws server
    void insert(gsl::span<std::byte> bytes);
    virtual std::unique_ptr<LwsPacket> message();
    void handleMessage(const LwsPacket& message);
    std::optional<LwsPacket> dataToSend();
    bool hasDataToSend();

    void handleDisconnect();
    //--

    void send(const LwsPacket& message);
    void sendHttpResult(const HttpRequestResult& result);

    std::shared_ptr<WsServerClientLws> m_publicClient;

    std::mutex m_mutex;
    std::list<LwsPacket> m_sendList;

    lws* m_wsi {nullptr};
    lws_context* m_serverContext {nullptr};

    std::unique_ptr<std::vector<std::byte>> m_receiveBuffer;

    std::string m_url;
    HttpMethod m_method;

    std::shared_ptr<IncomingHttpRequest> m_request;
};

} // namespace arista

#endif
