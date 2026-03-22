#pragma once

#include <optional>
#include <string>
#include <utility>

struct Exchange {
    std::string userQuery;
    std::string assistantReply;
};

class HistoryManager {
  public:
    explicit HistoryManager(std::string path = "~/.cache/how/history");

    /// Load the previous exchange, if any.
    [[nodiscard]] std::optional<Exchange> loadPrevious() const;

    /// Save the current exchange, creating the directory and enforcing 0600.
    void save(const std::string& userQuery, const std::string& assistantReply) const;

  private:
    [[nodiscard]] std::string resolvePath() const;

    std::string path_;
};
