#include "Provider.h"
#include "LLMClient.h" // for Message

#include <stdexcept>

Provider::Provider(std::string name, std::string endpoint, std::string apiKey, std::string model,
                   Format format)
    : name_(std::move(name)), endpoint_(std::move(endpoint)), apiKey_(std::move(apiKey)),
      model_(std::move(model)), format_(format) {}

Provider Provider::create(const std::string& name, const std::string& apiKey,
                          const std::string& model, const std::string& customEndpoint) {
    if (name == "openai") {
        return Provider(name, "https://api.openai.com/v1/chat/completions", apiKey,
                        model.empty() ? "gpt-5.4-nano" : model, Format::OpenAI);
    }
    if (name == "mistral") {
        return Provider(name, "https://api.mistral.ai/v1/chat/completions", apiKey,
                        model.empty() ? "mistral-small-latest" : model, Format::OpenAI);
    }
    if (name == "anthropic") {
        return Provider(name, "https://api.anthropic.com/v1/messages", apiKey,
                        model.empty() ? "claude-haiku-4-5-20251001" : model, Format::Anthropic);
    }
    if (name == "google") {
        const std::string m = model.empty() ? "gemini-2.5-flash-lite" : model;
        const std::string ep = "https://generativelanguage.googleapis.com/v1beta/models/" + m +
                               ":generateContent?key=" + apiKey;
        return Provider(name, ep, apiKey, m, Format::Google);
    }
    if (name == "custom") {
        if (customEndpoint.empty()) {
            throw std::runtime_error("custom provider requires custom_endpoint in config");
        }
        return Provider(name, customEndpoint, apiKey, model.empty() ? "default" : model,
                        Format::OpenAI);
    }

    throw std::runtime_error("Unknown provider: " + name +
                             ". Valid providers: openai, anthropic, mistral, google, custom");
}

std::string Provider::endpoint() const {
    return endpoint_;
}

const std::string& Provider::name() const {
    return name_;
}

std::vector<std::string> Provider::headers() const {
    std::vector<std::string> hdrs;
    hdrs.emplace_back("Content-Type: application/json");

    if (format_ == Format::OpenAI) {
        if (!apiKey_.empty()) {
            hdrs.emplace_back("Authorization: Bearer " + apiKey_);
        }
    } else if (format_ == Format::Anthropic) {
        hdrs.emplace_back("x-api-key: " + apiKey_);
        hdrs.emplace_back("anthropic-version: 2023-06-01");
    }
    // Google uses query param, no auth header needed

    return hdrs;
}

nlohmann::json Provider::formatRequest(const std::string& systemPrompt,
                                       const std::vector<Message>& messages) const {

    nlohmann::json payload;

    if (format_ == Format::OpenAI) {
        payload["model"] = model_;
        auto& msgs = payload["messages"];
        msgs.push_back({{"role", "system"}, {"content", systemPrompt}});
        for (const auto& msg : messages) {
            msgs.push_back({{"role", msg.role}, {"content", msg.content}});
        }
    } else if (format_ == Format::Anthropic) {
        payload["model"] = model_;
        payload["max_tokens"] = 1024;
        payload["system"] = systemPrompt;
        auto& msgs = payload["messages"];
        for (const auto& msg : messages) {
            msgs.push_back({{"role", msg.role}, {"content", msg.content}});
        }
    } else if (format_ == Format::Google) {
        payload["systemInstruction"] = {{"parts", {{{"text", systemPrompt}}}}};
        auto& contents = payload["contents"];
        for (const auto& msg : messages) {
            std::string role = (msg.role == "assistant") ? "model" : "user";
            contents.push_back({{"role", role}, {"parts", {{{"text", msg.content}}}}});
        }
    }

    return payload;
}

std::string Provider::parseResponse(const nlohmann::json& json) const {
    try {
        if (format_ == Format::OpenAI) {
            return json.at("choices").at(0).at("message").at("content").get<std::string>();
        }
        if (format_ == Format::Anthropic) {
            return json.at("content").at(0).at("text").get<std::string>();
        }
        if (format_ == Format::Google) {
            return json.at("candidates")
                .at(0)
                .at("content")
                .at("parts")
                .at(0)
                .at("text")
                .get<std::string>();
        }
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(std::string("Unexpected response format: ") + e.what());
    }
    throw std::runtime_error("Unexpected response format");
}
