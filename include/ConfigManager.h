#pragma once

#include <string>
#include <unordered_map>

class ConfigManager {
public:
    explicit ConfigManager(std::string path = "~/.config/how/config");

    void load();

    [[nodiscard]] std::string endpoint() const;
    [[nodiscard]] std::string apiKey() const;
    [[nodiscard]] bool isLoaded() const;

private:
    std::string resolvePath(const std::string& path) const;
    void checkPermissions(const std::string& resolved) const;
    void parseFile(const std::string& resolved);

    std::string path_;
    std::unordered_map<std::string, std::string> values_;
    bool loaded_ = false;
};
