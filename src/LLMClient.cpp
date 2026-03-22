#include "LLMClient.h"

#include <curl/curl.h>
#include <stdexcept>

LLMClient::LLMClient(std::string endpoint, std::string apiKey)
    : endpoint_(std::move(endpoint)), apiKey_(std::move(apiKey)) {}

size_t LLMClient::writeCallback(char* data, size_t size, size_t nmemb, std::string* out) {
    size_t totalBytes = size * nmemb;
    out->append(data, totalBytes);
    return totalBytes;
}

std::string LLMClient::complete(
    const std::string& systemPrompt,
    const std::vector<Message>& messages) const {

    // Build the request payload
    nlohmann::json payload;
    payload["model"] = "gpt-4o-mini";

    auto& msgs = payload["messages"];
    msgs.push_back({{"role", "system"}, {"content", systemPrompt}});
    for (const auto& msg : messages) {
        msgs.push_back({{"role", msg.role}, {"content", msg.content}});
    }

    std::string body = payload.dump();

    // Set up curl
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize curl");
    }

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string authHeader = "Authorization: Bearer " + apiKey_;
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, endpoint_.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
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
        std::string errMsg = json["error"].value("message", "Unknown API error");
        throw std::runtime_error("API error: " + errMsg);
    }

    try {
        return json.at("choices").at(0).at("message").at("content").get<std::string>();
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(std::string("Unexpected response format: ") + e.what());
    }
}
