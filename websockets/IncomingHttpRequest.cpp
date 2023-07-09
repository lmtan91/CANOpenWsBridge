#include "IncomingHttpRequest.hpp"

namespace arista {

IncomingHttpRequest::IncomingHttpRequest(const HttpRequest& request) :
    m_request(std::make_unique<HttpRequest>(request))
{}

void IncomingHttpRequest::insert(gsl::span<std::byte> bytes)
{
    if (!m_buffer) {
        m_buffer = std::make_unique<std::vector<std::byte>>();
    }

    m_buffer->insert(m_buffer->end(), bytes.begin(), bytes.end());
}

std::unique_ptr<HttpRequest> IncomingHttpRequest::completedRequest()
{
    if (m_buffer) {
        m_request->payload = *m_buffer;
    }
    auto result = std::move(m_request);
    m_buffer.reset();
    m_request.reset();
    return result;
}

} // namespace arista
