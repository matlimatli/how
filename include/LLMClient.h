#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct Message {
    std::string role;
    std::string content;
};

class LLMClient {
public:
    LLMClient(std::string endpoint, std::string apiKey);

    /// Send a chat completion request and return the assistant's reply.
    [[nodiscard]] std::string complete(
        const std::string& systemPrompt,
        const std::vector<Message>& messages) const;

private:
    static size_t writeCallback(char* data, size_t size, size_t nmemb, std::string* out);

    std::string endpoint_;
    std::string apiKey_;
};
