#include "ConfigManager.h"
#include "ContextGatherer.h"
#include "PromptBuilder.h"

#include <filesystem>
#include <iostream>
#include <sstream>

int main(int argc, char* argv[]) {
    // Build query from argv[0] command name + remaining arguments
    std::string command = std::filesystem::path(argv[0]).filename().string();
    std::ostringstream query;
    query << command;
    for (int i = 1; i < argc; ++i) {
        query << ' ' << argv[i];
    }

    // Gather system context
    ContextGatherer context;
    context.gather();

    // Load configuration
    ConfigManager config;
    try {
        config.load();
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    // Build the prompt that will be sent to the LLM
    PromptBuilder prompt;
    prompt.setQuery(query.str());
    prompt.setOS(context.os());
    prompt.setShell(context.shell());
    prompt.setWorkingDirectory(context.workingDirectory());

    std::cout << "=== System Prompt ===\n" << prompt.buildSystemPrompt() << "\n\n";
    std::cout << "=== User Message ===\n" << prompt.buildUserMessage() << '\n';

    return 0;
}
