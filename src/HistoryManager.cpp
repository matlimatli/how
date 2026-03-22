#include "HistoryManager.h"

#include <algorithm>
#include <ranges>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <unordered_set>

HistoryManager::HistoryManager(std::string path) : path_(std::move(path)) {}

std::string HistoryManager::resolvePath() const {
    if (path_.starts_with("~/")) {
        const char* home = std::getenv("HOME");
        if (home == nullptr) {
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
        std::time_t ts = json.value("timestamp", static_cast<std::time_t>(0));
        return Exchange{.userQuery = json.at("user").get<std::string>(),
                        .assistantReply = json.at("assistant").get<std::string>(),
                        .timestamp = ts};
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
    json["timestamp"] = std::time(nullptr);

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

namespace {

std::vector<std::string> splitAndLower(const std::string& text) {
    std::vector<std::string> words;
    std::istringstream stream(text);
    std::string word;
    while (stream >> word) {
        std::ranges::transform(word, word.begin(),
                               [](unsigned char c) { return std::tolower(c); });
        words.push_back(std::move(word));
    }
    return words;
}

} // namespace

bool HistoryManager::isFollowUp(const Exchange& previous, const std::string& currentQuery,
                                std::time_t now) {
    const double elapsed = std::difftime(now, previous.timestamp);
    constexpr double kRapidWindow = 120.0; // 2 minutes
    constexpr double kMaxWindow = 600.0;   // 10 minutes

    if (elapsed > kMaxWindow) {
        return false;
    }
    if (elapsed <= kRapidWindow) {
        return true;
    }

    // Between 2–10 minutes: check for follow-up signals
    auto words = splitAndLower(currentQuery);
    if (!words.empty()) {
        words.erase(words.begin()); // drop command name
    }

    if (words.size() <= 3) {
        return true;
    }

    static const std::unordered_set<std::string> signals = {
        "it",      "this",          "that",     "them",    "those",   "these",   "also",  "again",
        "instead", "but",           "more",     "another", "same",    "similar", "too",   "else",
        "however", "alternatively", "previous", "last",    "earlier", "before",  "above",
    };
    return std::ranges::any_of(words, [&](const std::string& w) { return signals.contains(w); });
}
