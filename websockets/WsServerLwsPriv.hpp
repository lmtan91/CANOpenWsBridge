#ifndef ARISTA_WEBSOCKET_LWS_PRIVATE_H
#define ARISTA_WEBSOCKET_LWS_PRIVATE_H

#include "Core.hpp"
#include "WorkerThread.hpp"

#include "WsServerClientLwsImpl.hpp"
#include "HttpRequest.hpp"

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace arista {

struct WsServerLwsPriv
{
    static std::tuple<HttpMethod, std::string> getMethod(lws* wsi);

    void addHttpClient(lws* wsi);
    void removeHttpClient(lws* wsi);

    void handleNewWebSocketClient(lws* wsi);
    void removeWebSocketClient(lws* wsi);

    // TODO use a promise for the bytes. So the body can be handles later
    void handleHttpRequest(const std::shared_ptr<WsServerClientLwsImpl>& client, const arista::HttpRequest& request);

    bool matchInclusiveUrl(std::string requestUrl, std::string subscribedUrl);

    static std::optional<http_status> toLwsStatusCode(HttpStatusCode statusCode);

    const lws_protocol_vhost_options pvo = { nullptr, nullptr, "websocket-server", "" };

    lws_http_mount mount {};
    lws_context_creation_info info {};
    lws_context* context { nullptr };

    std::vector<lws_protocols> protocols;

    struct HttpHandlers {
      std::string url;
      HttpMethod method {};
      std::vector<std::function<void(WsServerClient&, const HttpRequest&)>> callbacks;
    };

    struct WebSocketHandlers {
      std::string url;
      std::vector<std::function<void(WsServerClient&)>> callbacks;
    };

    std::vector<HttpHandlers> httpHandlers;
    std::vector<WebSocketHandlers> webSocketHandlers;

    std::map<void*, std::shared_ptr<WsServerClientLwsImpl>> httpClientsMap;
    std::map<void*, std::shared_ptr<WsServerClientLwsImpl>> webSocketClientsMap;
};



} // namespace arista

#endif
