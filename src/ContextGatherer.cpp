#include "ContextGatherer.h"

#include <cstdlib>
#include <filesystem>
#include <sys/utsname.h>

void ContextGatherer::gather() {
    struct utsname info{};
    if (uname(&info) == 0) {
        os_ = info.sysname;
        if (os_ == "Darwin") {
            os_ = "macOS";
        }
    }

    if (const char* shell = std::getenv("SHELL")) {
        shell_ = shell;
    }

    try {
        workingDirectory_ = std::filesystem::current_path().string();
    } catch (const std::filesystem::filesystem_error&) {
        // Keep default "unknown"
    }
}

std::string ContextGatherer::os() const {
    return os_;
}

std::string ContextGatherer::shell() const {
    return shell_;
}

std::string ContextGatherer::workingDirectory() const {
    return workingDirectory_;
}
