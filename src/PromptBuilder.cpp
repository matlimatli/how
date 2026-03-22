#include "PromptBuilder.h"

void PromptBuilder::setQuery(const std::string& query) { query_ = query; }
void PromptBuilder::setOS(const std::string& os) { os_ = os; }
void PromptBuilder::setShell(const std::string& shell) { shell_ = shell; }
void PromptBuilder::setWorkingDirectory(const std::string& dir) { workingDirectory_ = dir; }

std::string PromptBuilder::buildSystemPrompt() const {
    return
        "You are a command-line assistant. The user is working on a "
        + os_ + " system, using " + shell_ + ", in the directory " + workingDirectory_ + ".\n"
        "\n"
        "Rules:\n"
        "- Give a brief explanation (1-3 sentences max).\n"
        "- Then show the exact command(s) to run, each on its own line inside a single fenced code block (```).\n"
        "- If multiple commands are needed, list them in order inside the same code block.\n"
        "- The entire response must fit on a standard terminal screen (~24 lines). Be concise.\n"
        "- Tailor commands to the user's OS and shell. Do not suggest commands that won't work in their environment.\n"
        "- No preamble, no pleasantries, no follow-up questions.";
}

std::string PromptBuilder::buildUserMessage() const {
    return query_;
}
