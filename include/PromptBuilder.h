#pragma once

#include <string>

class PromptBuilder {
  public:
    void setQuery(const std::string& query);
    void setOS(const std::string& os);
    void setShell(const std::string& shell);
    void setWorkingDirectory(const std::string& dir);

    [[nodiscard]] std::string buildSystemPrompt() const;
    [[nodiscard]] std::string buildUserMessage() const;

  private:
    std::string query_;
    std::string os_;
    std::string shell_;
    std::string workingDirectory_;
};
