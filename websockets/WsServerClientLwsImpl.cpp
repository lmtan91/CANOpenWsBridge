#include "WsServerClientLwsImpl.hpp"

#include "Core.hpp"
#include "WsServerClientLws.hpp"
#include "WsServerLwsPriv.hpp"

namespace arista {

const int PacketSize = 65550;

WsServerClientLwsImpl::WsServerClientLwsImpl(lws_context* serverContext, lws* wsi)
    : m_wsi(wsi), m_serverContext(serverContext)
{
    m_publicClient = std::make_shared<WsServerClientLws>(this);

    auto [method, url] = WsServerLwsPriv::getMethod(wsi);
    m_method = method;
    m_url = url;
}

WsServerClientLwsImpl::~WsServerClientLwsImpl() {}

void WsServerClientLwsImpl::insert(gsl::span<std::byte> bytes)
{
    if (!m_receiveBuffer) {
        m_receiveBuffer = std::make_unique<std::vector<std::byte>>();
    }

    m_receiveBuffer->insert(m_receiveBuffer->end(), bytes.begin(), bytes.end());
}

std::unique_ptr<LwsPacket> WsServerClientLwsImpl::message()
{
    auto message = std::make_unique<LwsPacket>();
    message->payload = std::move(m_receiveBuffer);

    m_receiveBuffer.reset();
    return message;
}

std::optional<LwsPacket> WsServerClientLwsImpl::dataToSend()
{
    std::scoped_lock lock(m_mutex);

    if (m_sendList.empty()) {
        return {};
    }

    auto result = m_sendList.front();
    m_sendList.pop_front();
    return result;
}

bool WsServerClientLwsImpl::hasDataToSend()
{
    std::scoped_lock lock(m_mutex);
    return !m_sendList.empty();
}

void WsServerClientLwsImpl::send(const LwsPacket& message)
{
    // chop message in multiple packets if it's bigger then PacketSize
    if (message.payload->size() > PacketSize) {
        int idx = 0;
        while (idx < message.payload->size()) {
            LwsPacket sendMessage {};
            sendMessage.statusCode = message.statusCode;
            sendMessage.contentType = message.contentType;
            sendMessage.contentLength = message.payload->size();
            sendMessage.headers = message.headers;
            sendMessage.isFirst = idx == 0;

            int size = message.payload->size() - idx;
            if (size > PacketSize) {
                size = PacketSize;
            }

            // libwebsockets want's to have LWS_PRE bytes prepended for efficiency
            sendMessage.payload = std::make_unique<std::vector<std::byte>>(LWS_PRE + size);
            std::copy(message.payload->begin() + idx,
                      message.payload->begin() + idx + size,
                      sendMessage.payload->begin() + LWS_PRE);

            idx += PacketSize;
            sendMessage.isFinal = idx >= message.payload->size();
            sendMessage.isBinary = message.isBinary;

            {
                std::scoped_lock lock(m_mutex);
                m_sendList.push_back(sendMessage);
            }
        }
    } else {
        std::scoped_lock lock(m_mutex);

        LwsPacket sendMessage {};
        sendMessage.statusCode = message.statusCode;
        sendMessage.contentType = message.contentType;
        sendMessage.contentLength = message.payload->size();
        sendMessage.headers = message.headers;
        sendMessage.isFirst = true;
        sendMessage.isFinal = true;
        sendMessage.isBinary = message.isBinary;

        // libwebsockets want's to have LWS_PRE bytes prepended for efficiency
        sendMessage.payload = std::make_unique<std::vector<std::byte>>(LWS_PRE + message.payload->size());
        std::copy(message.payload->begin(), message.payload->end(), sendMessage.payload->begin() + LWS_PRE);

        m_sendList.push_back(sendMessage);
    }

    // Cancel poll so we can ask for writable callback
    lws_cancel_service(m_serverContext);
}

void WsServerClientLwsImpl::sendHttpResult(const HttpRequestResult& result)
{
    arista::LwsPacket packet {};
    packet.statusCode = result.statusCode;
    packet.contentType = result.contentType;
    packet.headers = result.headers;
    packet.payload = std::make_shared<std::vector<std::byte>>(result.payload); // copy data
    packet.isBinary = true;

    send(packet);
}

void WsServerClientLwsImpl::handleMessage(const LwsPacket& message) { m_publicClient->handleMessage(message); }

void WsServerClientLwsImpl::handleDisconnect()
{
    if (m_publicClient) {
        m_publicClient->handleDisconnect();
    }
}

} // namespace arista