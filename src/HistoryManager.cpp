#include "HistoryManager.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <sys/stat.h>
#include <nlohmann/json.hpp>

HistoryManager::HistoryManager(std::string path)
    : path_(std::move(path)) {}

std::string HistoryManager::resolvePath() const {
    if (path_.starts_with("~/")) {
        const char* home = std::getenv("HOME");
        if (!home) {
            throw std::runtime_error("HOME environment variable is not set");
        }
        return std::string(home) + path_.substr(1);
    }
    return path_;
}

std::optional<Exchange> HistoryManager::loadPrevious() const {
    auto resolved = resolvePath();

    std::ifstream file(resolved);
    if (!file.is_open()) {
        return std::nullopt;
    }

    try {
        auto json = nlohmann::json::parse(file);
        return Exchange{
            json.at("user").get<std::string>(),
            json.at("assistant").get<std::string>()
        };
    } catch (const nlohmann::json::exception&) {
        return std::nullopt;
    }
}

void HistoryManager::save(const std::string& userQuery, const std::string& assistantReply) const {
    auto resolved = resolvePath();

    // Ensure the parent directory exists
    std::filesystem::create_directories(std::filesystem::path(resolved).parent_path());

    nlohmann::json json;
    json["user"] = userQuery;
    json["assistant"] = assistantReply;

    std::ofstream file(resolved, std::ios::trunc);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot write history file: " + resolved);
    }
    file << json.dump(2) << '\n';
    file.close();

    // Enforce 0600 permissions
    if (chmod(resolved.c_str(), 0600) != 0) {
        throw std::runtime_error("Cannot set permissions on history file: " + resolved);
    }
}
