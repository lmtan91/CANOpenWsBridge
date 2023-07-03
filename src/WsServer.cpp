#include "WsServer.h"

namespace arista {

using namespace std::chrono_literals;

static int callbackWebsocketServer(lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len)
{
    
    return 0;
}

WsServer::WsServer() :
     m_workerThread("WebSocketServerLws", arista::Thread::Priority::Normal)
{
    mount.mountpoint = "/";               // mountpoint URL
    mount.origin = "./";                  // serve from dir
    mount.def = "index.html";             // default filename
    mount.origin_protocol = LWSMPRO_FILE; // files in a dir
    mount.mountpoint_len = 1;             // char count

    protocols.push_back({ "websocket-server",
                             callbackWebsocketServer,
                             0,
                             1024,
                             3,
                             nullptr,
                             65550 }); // TODO
    protocols.push_back({ nullptr, nullptr, 0, 0 } /* terminator */);
}

WsServer::~WsServer()
{
    stop();

    if (context) {
        lws_context_destroy(context);
    }
}

void WsServer::init(int port)
{
    info.port = port;
    info.protocols = protocols.data();
    info.pvo = &pvo;
    info.pt_serv_buf_size = 1024 * 1024 * 8; // TODO??

    info.mounts = &mount;
    info.error_document_404 = "/404.html";

    // Use this when the server is on internet
    //info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE; // NOLINT(hicpp-signed-bitwise)

    // info.user = static_cast<void*>(d);

    context = lws_create_context(&info);
    if (!context) {
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

void WsServer::stop()
{
    m_workerThread.stop();
    wakeup();

    if (!context) {
        return;
    }

    lws_context_destroy(context);
    context = nullptr;
}

void WsServer::join()
{
    m_workerThread.join();
}

void WsServer::wakeup() {
    if (context) {
        lws_cancel_service(context);
    }
}

bool WsServer::work() {

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