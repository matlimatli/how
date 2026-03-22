#include "ConfigManager.h"
#include "ContextGatherer.h"
#include "HistoryManager.h"
#include "LLMClient.h"
#include "PromptBuilder.h"
#include "Provider.h"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <vector>

int main(int argc, char* argv[]) {
    // Build query from argv[0] command name + remaining arguments
    const std::string command = std::filesystem::path(argv[0]).filename().string();
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

    // Resolve provider
    const std::string providerName = config.provider();
    Provider provider;
    try {
        provider = Provider::create(providerName, config.apiKey(providerName),
                                    config.model(providerName), config.customEndpoint());
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    // Build the prompt
    PromptBuilder prompt; // NOLINT(misc-const-correctness) - setters called below
    prompt.setQuery(query.str());
    prompt.setOS(context.os());
    prompt.setShell(context.shell());
    prompt.setWorkingDirectory(context.workingDirectory());

    // Assemble message history
    const HistoryManager history;
    std::vector<Message> messages;

    auto previous = history.loadPrevious();
    if (previous) {
        messages.push_back({.role = "user", .content = previous->userQuery});
        messages.push_back({.role = "assistant", .content = previous->assistantReply});
    }
    messages.push_back({.role = "user", .content = prompt.buildUserMessage()});

    // Send to LLM
    const LLMClient client(provider, config.allowInsecureSsl());
    std::string reply;
    try {
        reply = client.complete(prompt.buildSystemPrompt(), messages);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    std::cout << reply << '\n';

    // Cache this exchange for follow-up context
    try {
        history.save(prompt.buildUserMessage(), reply);
    } catch (const std::runtime_error& e) {
        std::cerr << "Warning: " << e.what() << '\n';
    }

    return 0;
}
