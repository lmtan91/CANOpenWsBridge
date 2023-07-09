
#include "WsServerClientLws.hpp"
#include "WsServerClientLwsImpl.hpp"

namespace arista {

WsServerClientLws::WsServerClientLws(WsServerClientLwsImpl* d_) : d(d_) {}

WsServerClientLws::~WsServerClientLws() {}

void WsServerClientLws::registerMessageHandler(std::function<void(const LwsPacket&)> callback)
{
    while (m_messageHandlers.find(++m_messageHandlerId) != m_messageHandlers.end()) {
        m_messageHandlerId++;
    }

    m_messageHandlers[m_messageHandlerId] = callback;
}

void WsServerClientLws::handleMessage(const LwsPacket& message)
{
    for (auto& handler : m_messageHandlers) {
        handler.second(message);
    }
}
void WsServerClientLws::send(const LwsPacket& message) { d->send(message); }

void WsServerClientLws::sendHttpResult(const HttpRequestResult& result) { d->sendHttpResult(result); }

void WsServerClientLws::handleDisconnect() {
    m_onDisconnectSubject.next();
}

} // namespace arista