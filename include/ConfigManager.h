#pragma once

#include <string>
#include <unordered_map>

class ConfigManager {
public:
    explicit ConfigManager(std::string path = "~/.config/how/config");

    void load();

    /// Active provider: HOW_PROVIDER env var, falling back to default_provider in config.
    [[nodiscard]] std::string provider() const;
    [[nodiscard]] std::string apiKey(const std::string& provider) const;
    [[nodiscard]] std::string model(const std::string& provider) const;
    [[nodiscard]] std::string customEndpoint() const;
    [[nodiscard]] bool isLoaded() const;

private:
    std::string resolvePath(const std::string& path) const;
    void checkPermissions(const std::string& resolved) const;
    void parseFile(const std::string& resolved);
    [[nodiscard]] std::string get(const std::string& key, const std::string& fallback = "") const;

    std::string path_;
    std::unordered_map<std::string, std::string> values_;
    bool loaded_ = false;
};
