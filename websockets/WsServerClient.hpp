
#ifndef ARISTA_WEBSOCKET_SERVER_CLIENT_H
#define ARISTA_WEBSOCKET_SERVER_CLIENT_H

#include "HttpRequest.hpp"

#include "Core.hpp"
#include "Subject.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace arista {

struct LwsPacket
{
    HttpStatusCode statusCode {};
    std::string contentType;
    std::map<std::string, std::string> headers;

    std::shared_ptr<std::vector<std::byte>> payload;
    int contentLength {};
    bool isBinary {};
    bool isFirst {};
    bool isFinal {};
};

class WsServerClient {
  public:
    virtual void send(const LwsPacket& message) = 0;
    virtual void sendHttpResult(const HttpRequestResult& result) = 0;

    virtual void registerMessageHandler(std::function<void(const LwsPacket&)> callback) = 0;

    virtual std::unique_ptr<arista::Subscription> onDisconnect(std::function<void(void)> callback) = 0;
};

} // namespace arista

#endif
