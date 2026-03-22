#include "PromptBuilder.h"

void PromptBuilder::setQuery(const std::string& query) { query_ = query; }
void PromptBuilder::setOS(const std::string& os) { os_ = os; }
void PromptBuilder::setShell(const std::string& shell) { shell_ = shell; }
void PromptBuilder::setWorkingDirectory(const std::string& dir) { workingDirectory_ = dir; }

std::string PromptBuilder::buildSystemPrompt() const {
    return
        "You are a concise terminal assistant. The user is working on a "
        + os_ + " system, using " + shell_ + ", in the directory " + workingDirectory_ + ".\n"
        "\n"
        "Rules:\n"
        "- The entire response must fit on a standard terminal screen (~24 lines). Be concise.\n"
        "- If the question involves a task that can be done with commands, give a brief explanation "
        "(1-3 sentences) followed by the exact command(s) in a single fenced code block (```). "
        "Tailor commands to the user's OS and shell.\n"
        "- If the question is general knowledge and no command is relevant, just answer it briefly. "
        "Do not force a command-line example where none is needed.\n"
        "- If you do not know the answer, say so. Do not guess or fabricate information.\n"
        "- No preamble, no pleasantries, no follow-up questions.";
}

std::string PromptBuilder::buildUserMessage() const {
    return query_;
}
