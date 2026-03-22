#pragma once

#include <string>

class ContextGatherer {
  public:
    void gather();

    [[nodiscard]] std::string os() const;
    [[nodiscard]] std::string shell() const;
    [[nodiscard]] std::string workingDirectory() const;

  private:
    std::string os_ = "unknown";
    std::string shell_ = "unknown";
    std::string workingDirectory_ = "unknown";
};
