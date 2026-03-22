#pragma once

#include "Provider.h"

#include <string>
#include <vector>

struct Message {
    std::string role;
    std::string content;
};

class LLMClient {
  public:
    explicit LLMClient(const Provider& provider, bool allowInsecureSsl = false);

    /// Send a chat completion request and return the assistant's reply.
    [[nodiscard]] std::string complete(const std::string& systemPrompt,
                                       const std::vector<Message>& messages) const;

  private:
    static size_t writeCallback(char* data, size_t size, size_t nmemb, std::string* out);

    const Provider& provider_;
    bool allowInsecureSsl_;
};
