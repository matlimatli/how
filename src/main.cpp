#include "ConfigManager.h"
#include "ContextGatherer.h"

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

    std::cout << "Query: " << query.str() << '\n';

    // Gather system context
    ContextGatherer context;
    context.gather();
    std::cout << "OS: " << context.os() << '\n';
    std::cout << "Shell: " << context.shell() << '\n';
    std::cout << "Directory: " << context.workingDirectory() << '\n';

    // Load configuration
    ConfigManager config;
    try {
        config.load();
        std::cout << "Config: loaded (endpoint: " << config.endpoint() << ")\n";
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
