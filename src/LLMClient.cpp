#include "LLMClient.h"

#include <curl/curl.h>
#include <stdexcept>

LLMClient::LLMClient(const Provider& provider, bool allowInsecureSsl)
    : provider_(provider), allowInsecureSsl_(allowInsecureSsl) {}

size_t LLMClient::writeCallback(char* data, size_t size, size_t nmemb, std::string* out) {
    const size_t totalBytes = size * nmemb;
    out->append(data, totalBytes);
    return totalBytes;
}

std::string LLMClient::complete(const std::string& systemPrompt,
                                const std::vector<Message>& messages) const {

    const std::string body = provider_.formatRequest(systemPrompt, messages).dump();

    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        throw std::runtime_error("Failed to initialize curl");
    }

    std::string response;
    struct curl_slist* headers = nullptr;
    for (const auto& hdr : provider_.headers()) {
        headers = curl_slist_append(headers, hdr.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_setopt(curl, CURLOPT_URL, provider_.endpoint().c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

    if (allowInsecureSsl_) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    } else {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    }

    const CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        if (res == CURLE_OPERATION_TIMEDOUT) {
            throw std::runtime_error("Request timed out");
        }
        throw std::runtime_error(std::string("Network error: ") + curl_easy_strerror(res));
    }

    // Parse the response
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error(std::string("Invalid JSON response: ") + e.what());
    }

    if (json.contains("error")) {
        const std::string errMsg = json["error"].value("message", "Unknown API error");
        throw std::runtime_error("API error: " + errMsg);
    }

    return provider_.parseResponse(json);
}
