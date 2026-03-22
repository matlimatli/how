#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

struct Message;

class Provider {
  public:
    Provider() = default;

    /// Create a provider by name. Throws if name is unknown.
    static Provider create(const std::string& name, const std::string& apiKey,
                           const std::string& model, const std::string& customEndpoint = "");

    [[nodiscard]] std::string endpoint() const;
    [[nodiscard]] std::vector<std::string> headers() const;
    [[nodiscard]] nlohmann::json formatRequest(const std::string& systemPrompt,
                                               const std::vector<Message>& messages) const;
    [[nodiscard]] std::string parseResponse(const nlohmann::json& json) const;
    [[nodiscard]] const std::string& name() const;

  private:
    enum class Format { OpenAI, Anthropic, Google };

    Provider(std::string name, std::string endpoint, std::string apiKey, std::string model,
             Format format);

    std::string name_;
    std::string endpoint_;
    std::string apiKey_;
    std::string model_;
    Format format_;
};
