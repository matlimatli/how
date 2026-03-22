#pragma once

#include <ctime>
#include <optional>
#include <string>
#include <utility>

struct Exchange {
    std::string userQuery;
    std::string assistantReply;
    std::time_t timestamp{0};
};

class HistoryManager {
  public:
    explicit HistoryManager(std::string path = "~/.cache/how/history");

    /// Load the previous exchange, if any.
    [[nodiscard]] std::optional<Exchange> loadPrevious() const;

    /// Save the current exchange, creating the directory and enforcing 0600.
    void save(const std::string& userQuery, const std::string& assistantReply) const;

    /// Decide whether the current query is a follow-up to a previous exchange.
    [[nodiscard]] static bool isFollowUp(const Exchange& previous, const std::string& currentQuery,
                                         std::time_t now = std::time(nullptr));

  private:
    [[nodiscard]] std::string resolvePath() const;

    std::string path_;
};
