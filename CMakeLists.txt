cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0135 NEW)
project(HowUtility VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Strict compiler warnings
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# Optional sanitizers for debug builds
option(USE_SANITIZERS "Enable ASan and UBSan" OFF)
if(USE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# Find libcurl
find_package(CURL QUIET)
if(NOT CURL_FOUND)
    message(FATAL_ERROR
        "libcurl development headers not found.\n"
        "Install them with:\n"
        "  Ubuntu/Debian:  sudo apt install libcurl4-openssl-dev\n"
        "  Fedora/RHEL:    sudo dnf install libcurl-devel\n"
        "  macOS (Homebrew): brew install curl\n"
        "  macOS (system):   usually pre-installed — try: xcode-select --install\n"
    )
endif()

# Fetch dependencies (downloads may take a moment on first configure)
include(FetchContent)
message(STATUS "Fetching nlohmann/json 3.12.0...")
set(JSON_Install OFF CACHE BOOL "" FORCE)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/archive/refs/tags/v3.12.0.tar.gz
)
FetchContent_MakeAvailable(json)

# Core library (shared between executable and tests)
add_library(how_core STATIC
    src/ConfigManager.cpp
    src/ContextGatherer.cpp
    src/PromptBuilder.cpp
    src/LLMClient.cpp
    src/HistoryManager.cpp
    src/Provider.cpp
)
target_include_directories(how_core PUBLIC include)
target_link_libraries(how_core PUBLIC CURL::libcurl nlohmann_json::nlohmann_json)

# Executable
add_executable(how src/main.cpp)
target_link_libraries(how PRIVATE how_core)

# Tests
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    message(STATUS "Fetching googletest 1.17.0...")
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(how_tests
        tests/ConfigManagerTest.cpp
        tests/HistoryManagerTest.cpp
    )
    target_link_libraries(how_tests PRIVATE how_core GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(how_tests)

    # 'make test' rebuilds before running tests
    add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        DEPENDS how_tests
    )
endif()

# Optional clang-tidy target
file(GLOB_RECURSE ALL_SOURCES src/*.cpp include/*.h)
find_program(CLANG_TIDY clang-tidy
    HINTS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin)
if(CLANG_TIDY)
    # When using Homebrew clang-tidy with Apple Clang, pass the SDK path
    set(CLANG_TIDY_EXTRA_ARGS "")
    if(APPLE)
        execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE SDKROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
        set(CLANG_TIDY_EXTRA_ARGS "--extra-arg=-isysroot${SDKROOT}")
    endif()
    add_custom_target(tidy
        COMMAND ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR} ${CLANG_TIDY_EXTRA_ARGS} ${ALL_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy..."
    )
endif()

# Optional clang-format target
find_program(CLANG_FORMAT clang-format
    HINTS /opt/homebrew/opt/llvm/bin /usr/local/opt/llvm/bin)
if(CLANG_FORMAT)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i ${ALL_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-format..."
    )
    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT} --dry-run --Werror ${ALL_SOURCES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Checking clang-format..."
    )
endif()

# Install
include(GNUInstallDirs)
install(TARGETS how RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES man/how.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

# Create symlinks for alternative command names
foreach(alias what when why)
    install(CODE "
        execute_process(
            COMMAND \${CMAKE_COMMAND} -E create_symlink how
                \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/${alias}
        )
        message(STATUS \"Symlink: ${alias} -> how\")
    ")
endforeach()

# --- CPack packaging ---
set(CPACK_PACKAGE_NAME "how")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Ask an LLM questions from the terminal")
set(CPACK_PACKAGE_DESCRIPTION "A one-shot CLI utility that sends a question to a large language model and prints the answer. The command name forms the start of the query.")
set(CPACK_PACKAGE_CONTACT "Mattias Lindblad")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/matlimatli/how")

# Debian
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libcurl4")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_SOURCE_DIR}/packaging/postinst;${CMAKE_SOURCE_DIR}/packaging/postrm")

include(CPack)
