#include "WsServerLws.hpp"
#include "WsServerLwsPriv.hpp"


#include "WsServerClientLwsImpl.hpp"

#include <libwebsockets.h>
#include <variant>

namespace arista {

using namespace std::chrono_literals;

int handleHttpNewConnection(lws* wsi, WsServerLwsPriv* server)
{
    auto* vhd = (VhdWsServer*)lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));

    if (!vhd) {
        return 0;
    }

    server->addHttpClient(wsi);
    auto& client = server->httpClientsMap[static_cast<void*>(wsi)];

    HttpRequest request;

    auto [method, url] = WsServerLwsPriv::getMethod(wsi);
    request.method = method;
    request.url = url;

    if (int length = lws_hdr_total_length(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE); length > 0) {
        std::string contentType(length+1, 0);
        lws_hdr_copy(wsi, contentType.data(), length + 1, WSI_TOKEN_HTTP_CONTENT_TYPE);
        contentType.resize(length);

        request.contentType = contentType;
    }

    if (int length = lws_hdr_total_length(wsi, WSI_TOKEN_HTTP_CONTENT_LENGTH); length > 0) {
        std::string contentLengthString(length+1, 0);
        lws_hdr_copy(wsi, contentLengthString.data(), length + 1, WSI_TOKEN_HTTP_CONTENT_LENGTH);
        contentLengthString.resize(length);

        try {
            request.contentLength = std::stoi(contentLengthString);
        } catch(...) {
            request.contentLength = 0;
            arista::printError("Error content length: {}", contentLengthString);
        }
    }

    client->m_request = std::make_shared<IncomingHttpRequest>(request);

    if (!request.contentLength) {
        auto request = client->m_request->completedRequest();
        server->handleHttpRequest(client, *request);

        if (client->hasDataToSend()) {
            lws_callback_on_writable(wsi);
        } else {
            return 1;
        }

    }

    return 0;
}

static int callbackWebsocketServer(lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len)
{

    auto ctx = lws_get_context(wsi);
    void* cuser = lws_context_user(ctx);

    auto* webSocketServer = reinterpret_cast<WsServerLwsPriv*>(cuser);

    switch (reason) {

    case LWS_CALLBACK_PROTOCOL_INIT: {
        auto* vhd = reinterpret_cast<VhdWsServer*>(
            lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi), lws_get_protocol(wsi), sizeof(VhdWsServer)));

        if (!vhd)
            return -1;

        vhd->context = lws_get_context(wsi);
        vhd->vhost = lws_get_vhost(wsi);
        break;
    }

    case LWS_CALLBACK_PROTOCOL_DESTROY:
        break;
    case LWS_CALLBACK_WSI_CREATE:
        break;
    case LWS_CALLBACK_WSI_DESTROY:
        break;

    case LWS_CALLBACK_HTTP:ll
        if (handleHttpNewConnection(wsi, webSocketServer)) {
            return 1;
        }

        break;

    case LWS_CALLBACK_HTTP_BODY: {
        auto* vhd = (VhdWsServer*)lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));

        gsl::span<std::byte> bytes(reinterpret_cast<std::byte*>(in), len);

        auto& client = webSocketServer->httpClientsMap[static_cast<void*>(wsi)];

        client->m_request->insert(bytes);

        break;
    }
    case LWS_CALLBACK_HTTP_BODY_COMPLETION: {
        auto* vhd = (VhdWsServer*)lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));

        auto& client = webSocketServer->httpClientsMap[static_cast<void*>(wsi)];

        auto request = client->m_request->completedRequest();

        webSocketServer->handleHttpRequest(client, *request);

        if (client->hasDataToSend()) {
            lws_callback_on_writable(wsi);
        } else {
            return 1;
        }

        break;
    }
    case LWS_CALLBACK_HTTP_WRITEABLE: {
        auto* vhd = (VhdWsServer*)lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));

        auto& client = webSocketServer->httpClientsMap[static_cast<void*>(wsi)];

        auto dataOpt = client->dataToSend();
        if (!dataOpt) {
            arista::printError("nothing to send!");
            return 0;
        }
        auto data = dataOpt.value();

        std::vector<std::byte> bytes(LWS_PRE + 1000);
        
        uint8_t *start = (uint8_t*)bytes.data() + LWS_PRE;
        uint8_t *end =  (uint8_t*)bytes.data() + 1000 + data.payload->size();
        uint8_t *p = start;

        http_status lwsHttpStatus = static_cast<http_status>(data.statusCode);
        auto statusCode = WsServerLwsPriv::toLwsStatusCode(data.statusCode);
        if (statusCode) {
            lwsHttpStatus = statusCode.value();
        } else {
            arista::printError("Unknown status code: {}", static_cast<int>(data.statusCode));
        }

        // TODO test multi-packet sending
        if (data.isFirst) {
            if (lws_add_http_common_headers(
                    wsi, lwsHttpStatus, data.contentType.c_str(), data.contentLength, &p, end)) {
                arista::printInfo("r1");

                return 1;
            }

            for(auto& pair: data.headers) {
                std::string name = pair.first + ":";
                auto bufferTooSmall = lws_add_http_header_by_name(wsi, reinterpret_cast<const unsigned char*>(name.c_str()), reinterpret_cast<const unsigned char*>(pair.second.c_str()), pair.second.size(), &p, end);
                if (bufferTooSmall) {
                    arista::printError("LWS: buffer too small for writing header_by_name!");
                }
            }

            if (lws_finalize_write_http_header(wsi, start, &p, end)) {
                arista::printError("error writing to socket!");
                return 1;
            }
        }

        if (data.payload) {
            int flags = lws_write_ws_flags(data.isBinary ? LWS_WRITE_BINARY : LWS_WRITE_TEXT, data.isFirst, data.isFinal);

            // payload already has LWS_PRE prepended
            int result = lws_write(wsi, (unsigned char*)data.payload->data() + LWS_PRE, data.payload->size() - LWS_PRE, (lws_write_protocol)flags);

            if (result < (data.payload->size() - LWS_PRE)) {
                arista::printError("error writing to socket!");
                return -1;
            }
        }

        if (!data.isFinal && client->hasDataToSend()) {
            lws_callback_on_writable(wsi);
        }

        // Need to close connection otherwise next request immediately gives LWS_CALLBACK_HTTP_BODY_COMPLETION
        if (data.isFinal) {
            return -1;
        }

        break;
    }
    case LWS_CALLBACK_CLOSED_HTTP: {
        webSocketServer->removeHttpClient(wsi);
        break;
    }
    case LWS_CALLBACK_ESTABLISHED: {
        webSocketServer->handleNewWebSocketClient(wsi);

        lws_callback_on_writable(wsi);
        break;
    }
    case LWS_CALLBACK_SERVER_WRITEABLE: {

        auto* vhd = (VhdWsServer*)lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));

        auto& client = webSocketServer->webSocketClientsMap[static_cast<void*>(wsi)];

        auto dataOpt = client->dataToSend();
        if (!dataOpt.has_value()) {
            // Nothing to write
            return 0;
        }
        auto data = dataOpt.value();

        if (data.payload) {
            int flags = lws_write_ws_flags(data.isBinary ? LWS_WRITE_BINARY : LWS_WRITE_TEXT, data.isFirst, data.isFinal);

            // payload already has LWS_PRE prepended
            int result = lws_write(wsi, (unsigned char*)data.payload->data() + LWS_PRE, data.payload->size() - LWS_PRE, (lws_write_protocol)flags);

            if (result < (data.payload->size() - LWS_PRE)) {
                arista::printError("error writing to socket!");
                return -1;
            }
        }

        if (client->hasDataToSend()) {
            lws_callback_on_writable(wsi);
        }

        break;
    }
    case LWS_CALLBACK_RECEIVE: {
        int remainingPackets = (int)lws_remaining_packet_payload(wsi);
        int isFirst = (int)lws_is_first_fragment(wsi);
        int isFinal = (int)lws_is_final_fragment(wsi);
        int isBinary = (int)lws_frame_is_binary(wsi);

        auto& client = webSocketServer->webSocketClientsMap[static_cast<void*>(wsi)];

        auto* vhd = (VhdWsServer*)lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));

        gsl::span<std::byte> bytes(reinterpret_cast<std::byte*>(in), len);
        client->insert(bytes);

        if (isFinal) {
            client->handleMessage(*client->message());
        }

        break;
    }
    case LWS_CALLBACK_CLOSED: {
        webSocketServer->removeWebSocketClient(wsi);
        break;
    }
    case LWS_CALLBACK_ADD_HEADERS:
        break;
    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
        break;
    case LWS_CALLBACK_HTTP_BIND_PROTOCOL:
        break;
    case LWS_CALLBACK_HTTP_CONFIRM_UPGRADE:
        break;
    case LWS_CALLBACK_WS_SERVER_DROP_PROTOCOL:
        break;
    case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
        break;
    case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
        break;
    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
        break;
    case LWS_CALLBACK_FILTER_HTTP_CONNECTION:
        break;
    case LWS_CALLBACK_GET_THREAD_ID:
        break;
    case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
        break;
    default:
        break;
    }

    return 0;
}


WsServerLws::WsServerLws() :
    m_workerThread("WsServerLws", arista::Thread::Priority::Normal)
{
    d = new WsServerLwsPriv();

    d->mount.mountpoint = "/";               // mountpoint URL
    d->mount.origin = "./";                  // serve from dir
    d->mount.def = "index.html";             // default filename
    d->mount.origin_protocol = LWSMPRO_FILE; // files in a dir
    d->mount.mountpoint_len = 1;             // char count

    d->protocols.push_back({ "websocket-server",
                             callbackWebsocketServer,
                             0,
                             1024,
                             3,
                             nullptr,
                             65550 }); // TODO
    d->protocols.push_back({ nullptr, nullptr, 0, 0 } /* terminator */);
}

WsServerLws::~WsServerLws()
{
    stop();

    if (d && d->context) {
        lws_context_destroy(d->context);
    }
    delete d;
    d = nullptr;
}

void WsServerLws::init(int port)
{
    d->info.port = port;
    d->info.protocols = d->protocols.data();
    d->info.pvo = &d->pvo;
    d->info.pt_serv_buf_size = 1024 * 1024 * 8; // TODO??

    d->info.mounts = &d->mount;
    d->info.error_document_404 = "/404.html";

    // Use this when the server is on internet
    //d->info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE; // NOLINT(hicpp-signed-bitwise)

    d->info.user = static_cast<void*>(d);

    d->context = lws_create_context(&d->info);
    if (!d->context) {
        arista::printError("lws init failed\n");
        return;
    }

    auto initResult = m_workerThread.init([this]() {
        return this->work();
    }, 0ms);

    if (!initResult) {
        arista::printError("creating workerthread failed!");
    }
}
void WsServerLws::stop()
{
    m_workerThread.stop();
    wakeup();

    if (!d->context) {
        return;
    }

    lws_context_destroy(d->context);
    d->context = nullptr;
}

void WsServerLws::join()
{
    m_workerThread.join();
}

void WsServerLws::handle(arista::HttpMethod method, const std::string& url, const std::function<void(WsServerClient&, const HttpRequest&)>& callback)
{
    for (auto& it : d->httpHandlers) {
        if (it.url != url || it.method != method) {
            continue;
        }

        it.callbacks.push_back(callback);
        return;
    }

    WsServerLwsPriv::HttpHandlers handler {};
    handler.url = url;
    handler.method = method;
    handler.callbacks.push_back(callback);
    d->httpHandlers.push_back(handler);
}

void WsServerLws::handleWebSocket(const std::string& url, const std::function<void(WsServerClient&)>& callback)
{
    for (auto& it : d->webSocketHandlers) {
        if (it.url != url) {
            continue;
        }

        it.callbacks.push_back(callback);
        return;
    }

    arista::printInfo("handleWebSocket handler!!");
    WsServerLwsPriv::WebSocketHandlers handler {};
    handler.url = url;
    handler.callbacks.push_back(callback);
    d->webSocketHandlers.push_back(handler);
}

void WsServerLws::wakeup() {
    if (d->context) {
        lws_cancel_service(d->context);
    }
}

bool WsServerLws::work() {

    for (auto& clientPair : d->httpClientsMap) {
        auto& client = clientPair.second;
        if (client->hasDataToSend()) {
            lws_callback_on_writable(client->m_wsi);
        }
    }
    for (auto& clientPair : d->webSocketClientsMap) {
        auto& client = clientPair.second;
        if (client->hasDataToSend()) {
            lws_callback_on_writable(client->m_wsi);
        }
    }

    int n = lws_service(d->context, 100);
    return true; // call work again immediately
}

} // namespace arista