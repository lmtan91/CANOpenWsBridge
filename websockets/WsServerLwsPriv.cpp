#include "WsServerLwsPriv.hpp"

#include "WsServerClientLwsImpl.hpp"
#include "WsServerClientLws.hpp"

#include "StringUtils.hpp"

#include <libwebsockets.h>
#include <variant>

namespace arista {

using namespace std::chrono_literals;

std::tuple<HttpMethod, std::string> WsServerLwsPriv::getMethod(lws* wsi)
{
    if (int length = lws_hdr_total_length(wsi, WSI_TOKEN_GET_URI); length > 0) {
        std::vector<char> uriVec(length + 1, 0);
        lws_hdr_copy(wsi, uriVec.data(), length + 1, WSI_TOKEN_GET_URI);
        return { HttpMethod::Get, std::string(uriVec.data()) };
    }
    if (int length = lws_hdr_total_length(wsi, WSI_TOKEN_POST_URI); length > 0) {
        std::vector<char> uriVec(length + 1, 0);
        lws_hdr_copy(wsi, uriVec.data(), length + 1, WSI_TOKEN_POST_URI);
        return { HttpMethod::Post, std::string(uriVec.data()) };
    }
    if (int length = lws_hdr_total_length(wsi, WSI_TOKEN_PUT_URI); length > 0) {
        std::vector<char> uriVec(length + 1, 0);
        lws_hdr_copy(wsi, uriVec.data(), length + 1, WSI_TOKEN_PUT_URI);
        return { HttpMethod::Put, std::string(uriVec.data()) };
    }
    if (int length = lws_hdr_total_length(wsi, WSI_TOKEN_OPTIONS_URI); length > 0) {
        std::vector<char> uriVec(length + 1, 0);
        lws_hdr_copy(wsi, uriVec.data(), length + 1, WSI_TOKEN_OPTIONS_URI);
        return { HttpMethod::Options, std::string(uriVec.data()) };
    }
    if (int length = lws_hdr_total_length(wsi, WSI_TOKEN_DELETE_URI); length > 0) {
        std::vector<char> uriVec(length + 1, 0);
        lws_hdr_copy(wsi, uriVec.data(), length + 1, WSI_TOKEN_DELETE_URI);
        return { HttpMethod::Delete, std::string(uriVec.data()) };
    }
    if (int length = lws_hdr_total_length(wsi, WSI_TOKEN_HEAD_URI); length > 0) {
        std::vector<char> uriVec(length + 1, 0);
        lws_hdr_copy(wsi, uriVec.data(), length + 1, WSI_TOKEN_HEAD_URI);
        return { HttpMethod::Head, std::string(uriVec.data()) };
    }
    if (int length = lws_hdr_total_length(wsi, WSI_TOKEN_PATCH_URI); length > 0) {
        std::vector<char> uriVec(length + 1, 0);
        lws_hdr_copy(wsi, uriVec.data(), length + 1, WSI_TOKEN_PATCH_URI);
        return { HttpMethod::Patch, std::string(uriVec.data()) };
    }

    return { HttpMethod::Unknown, "" };
}

void WsServerLwsPriv::addHttpClient(lws* wsi)
{
    auto client = std::make_shared<WsServerClientLwsImpl>(context, wsi);
    httpClientsMap[static_cast<void*>(wsi)] = client;
}

void WsServerLwsPriv::removeHttpClient(lws* wsi)
{
    auto& client = httpClientsMap[static_cast<void*>(wsi)];
    httpClientsMap.erase(static_cast<void*>(wsi));
}

void WsServerLwsPriv::handleNewWebSocketClient(lws* wsi) {
     arista::printInfo("handleNewWebSocketClient");
    auto client = std::make_shared<WsServerClientLwsImpl>(context, wsi);
    webSocketClientsMap[static_cast<void*>(wsi)] = client;

    bool found = false;
    for (auto& it : webSocketHandlers) {
        arista::printInfo("handleNewWebSocketClient {}:{}", it.url.c_str(), client->m_url.c_str());
        if (it.url != client->m_url) {
            continue;
        }

        found = true;

        for (auto& callbackIt : it.callbacks) {
            callbackIt(*client->m_publicClient);
        }
    }

    if (!found) {
        arista::printInfo("no handler found for url: {}", client->m_url);
    }
}

void WsServerLwsPriv::removeWebSocketClient(lws* wsi) {

    auto& client = webSocketClientsMap[static_cast<void*>(wsi)];

    client->handleDisconnect();

    webSocketClientsMap.erase(static_cast<void*>(wsi));

}

void WsServerLwsPriv::handleHttpRequest(const std::shared_ptr<WsServerClientLwsImpl>& client, const arista::HttpRequest& request) {
    
    bool found = false;
    for (auto& it : httpHandlers) {
        if (!matchInclusiveUrl(request.url, it.url) || it.method != request.method) {
            continue;
        }

        found = true;

        for (auto& callbackIt : it.callbacks) {
            callbackIt(*client->m_publicClient, request);
        }
    }

    // Match wildcard
    if (!found) {
        for (auto& it : httpHandlers) {
            if (it.url != "*" || it.method != request.method) {
                continue;
            }

            found = true;

            for (auto& callbackIt : it.callbacks) {
                callbackIt(*client->m_publicClient, request);
            }
        }
    }

    if (!found) {
        arista::printInfo("no handler found for url: {}", request.url);

        // TODO let client code handle this?

        HttpRequestResult result(request);
        result.request = request;
        result.statusCode = HttpStatusCode::NotFound;
        client->sendHttpResult(result);
    }
}

std::optional<http_status> WsServerLwsPriv::toLwsStatusCode(HttpStatusCode statusCode)
{
    switch (statusCode) {
    case HttpStatusCode::Continue: return HTTP_STATUS_CONTINUE;
    case HttpStatusCode::Ok: return HTTP_STATUS_OK;
    case HttpStatusCode::Created: return static_cast<http_status>(statusCode);
    case HttpStatusCode::BadRequest: return HTTP_STATUS_BAD_REQUEST;
    case HttpStatusCode::NotFound: return HTTP_STATUS_NOT_FOUND;
    case HttpStatusCode::InternalServerError: return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    case HttpStatusCode::ServiceUnavailable: return HTTP_STATUS_SERVICE_UNAVAILABLE;
    }

    return {};
}

bool WsServerLwsPriv::matchInclusiveUrl(std::string requestUrl, std::string subscribedUrl) {
    auto requestSplits = arista::string::split(requestUrl, "/");
    auto subscribedSplits = arista::string::split(subscribedUrl, "/");

    for(int i = 0; i < subscribedSplits.size(); i++) {
        // don't compare input variables
        if (arista::string::startsWith(subscribedSplits[i], ":")) {
            continue;
        }

        if (i < requestSplits.size() && subscribedSplits[i] != requestSplits[i]) {
            return false;
        }
    }

    return true;
}

} // namespace arista