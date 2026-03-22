#include "ConfigManager.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>

ConfigManager::ConfigManager(std::string path)
    : path_(std::move(path)) {}

std::string ConfigManager::resolvePath(const std::string& path) const {
    if (path.starts_with("~/")) {
        const char* home = std::getenv("HOME");
        if (!home) {
            throw std::runtime_error("HOME environment variable is not set");
        }
        return std::string(home) + path.substr(1);
    }
    return path;
}

void ConfigManager::checkPermissions(const std::string& resolved) const {
    struct stat st{};
    if (stat(resolved.c_str(), &st) != 0) {
        throw std::runtime_error("Cannot stat config file: " + resolved);
    }

    mode_t perms = st.st_mode & 0777;
    if (perms & ~static_cast<mode_t>(0600)) {
        std::ostringstream msg;
        msg << "Config file permissions are too open (0"
            << std::oct << perms << "). "
            << "Run: chmod 600 " << resolved;
        throw std::runtime_error(msg.str());
    }
}

void ConfigManager::parseFile(const std::string& resolved) {
    std::ifstream file(resolved);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open config file: " + resolved);
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        auto pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }

        auto key = line.substr(0, pos);
        auto value = line.substr(pos + 1);

        // Trim whitespace
        auto trim = [](std::string& s) {
            s.erase(0, s.find_first_not_of(" \t"));
            s.erase(s.find_last_not_of(" \t") + 1);
        };
        trim(key);
        trim(value);

        values_[key] = value;
    }
}

void ConfigManager::load() {
    auto resolved = resolvePath(path_);
    checkPermissions(resolved);
    parseFile(resolved);
    loaded_ = true;
}

std::string ConfigManager::endpoint() const {
    auto it = values_.find("endpoint");
    return it != values_.end() ? it->second : "";
}

std::string ConfigManager::apiKey() const {
    auto it = values_.find("api_key");
    return it != values_.end() ? it->second : "";
}

std::string ConfigManager::model() const {
    auto it = values_.find("model");
    return it != values_.end() ? it->second : "gpt-4o-mini";
}

bool ConfigManager::isLoaded() const {
    return loaded_;
}
