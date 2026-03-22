cmake_minimum_required(VERSION 3.20)
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
find_package(CURL REQUIRED)

# Fetch lightweight JavaScript Object Notation parser
include(FetchContent)
set(JSON_Install OFF CACHE BOOL "" FORCE)
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
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
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(how_tests
        tests/ConfigManagerTest.cpp
        tests/HistoryManagerTest.cpp
    )
    target_link_libraries(how_tests PRIVATE how_core GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(how_tests)
endif()

# Install
include(GNUInstallDirs)
install(TARGETS how RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

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
