#include "ConfigManager.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sys/stat.h>

class ConfigManagerTest : public ::testing::Test {
protected:
    std::filesystem::path tmpDir_;

    void SetUp() override {
        tmpDir_ = std::filesystem::temp_directory_path() / "how_test_config";
        std::filesystem::create_directories(tmpDir_);
    }

    void TearDown() override {
        std::filesystem::remove_all(tmpDir_);
    }

    std::string writeConfig(const std::string& content, mode_t perms = 0600) {
        auto path = tmpDir_ / "config";
        std::ofstream file(path, std::ios::trunc);
        file << content;
        file.close();
        chmod(path.c_str(), perms);
        return path.string();
    }
};

TEST_F(ConfigManagerTest, MissingFileThrows) {
    ConfigManager mgr("/nonexistent/path/config");
    EXPECT_THROW(mgr.load(), std::runtime_error);
}

TEST_F(ConfigManagerTest, InsecurePermissionsThrows) {
    auto path = writeConfig("key = value", 0644);
    ConfigManager mgr(path);
    try {
        mgr.load();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_NE(std::string(e.what()).find("too open"), std::string::npos);
        EXPECT_NE(std::string(e.what()).find("0644"), std::string::npos);
    }
}

TEST_F(ConfigManagerTest, WorldReadablePermissionsThrows) {
    auto path = writeConfig("key = value", 0604);
    ConfigManager mgr(path);
    EXPECT_THROW(mgr.load(), std::runtime_error);
}

TEST_F(ConfigManagerTest, GroupWritablePermissionsThrows) {
    auto path = writeConfig("key = value", 0620);
    ConfigManager mgr(path);
    EXPECT_THROW(mgr.load(), std::runtime_error);
}

TEST_F(ConfigManagerTest, CorrectPermissionsLoads) {
    auto path = writeConfig("default_provider = openai\n", 0600);
    ConfigManager mgr(path);
    EXPECT_NO_THROW(mgr.load());
    EXPECT_TRUE(mgr.isLoaded());
}

TEST_F(ConfigManagerTest, OwnerReadOnlyPermissionsLoads) {
    auto path = writeConfig("default_provider = openai\n", 0400);
    ConfigManager mgr(path);
    EXPECT_NO_THROW(mgr.load());
}

TEST_F(ConfigManagerTest, ParsesProviderConfig) {
    auto path = writeConfig(
        "default_provider = mistral\n"
        "mistral_api_key = test-key\n"
        "mistral_model = mistral-small-latest\n");
    ConfigManager mgr(path);
    mgr.load();
    EXPECT_EQ(mgr.provider(), "mistral");
    EXPECT_EQ(mgr.apiKey("mistral"), "test-key");
    EXPECT_EQ(mgr.model("mistral"), "mistral-small-latest");
}

TEST_F(ConfigManagerTest, AllowInsecureSslDefaultsFalse) {
    auto path = writeConfig("default_provider = openai\n");
    ConfigManager mgr(path);
    mgr.load();
    EXPECT_FALSE(mgr.allowInsecureSsl());
}

TEST_F(ConfigManagerTest, AllowInsecureSslTrue) {
    auto path = writeConfig("allow_insecure_ssl = true\n");
    ConfigManager mgr(path);
    mgr.load();
    EXPECT_TRUE(mgr.allowInsecureSsl());
}

TEST_F(ConfigManagerTest, AllowInsecureSslYes) {
    auto path = writeConfig("allow_insecure_ssl = yes\n");
    ConfigManager mgr(path);
    mgr.load();
    EXPECT_TRUE(mgr.allowInsecureSsl());
}

TEST_F(ConfigManagerTest, AllowInsecureSslOne) {
    auto path = writeConfig("allow_insecure_ssl = 1\n");
    ConfigManager mgr(path);
    mgr.load();
    EXPECT_TRUE(mgr.allowInsecureSsl());
}

TEST_F(ConfigManagerTest, AllowInsecureSslExplicitFalse) {
    auto path = writeConfig("allow_insecure_ssl = false\n");
    ConfigManager mgr(path);
    mgr.load();
    EXPECT_FALSE(mgr.allowInsecureSsl());
}

TEST_F(ConfigManagerTest, SkipsCommentsAndEmptyLines) {
    auto path = writeConfig(
        "# This is a comment\n"
        "\n"
        "default_provider = openai\n"
        "# Another comment\n");
    ConfigManager mgr(path);
    mgr.load();
    EXPECT_EQ(mgr.provider(), "openai");
}

TEST_F(ConfigManagerTest, TrimsWhitespace) {
    auto path = writeConfig("  openai_api_key  =  sk-test  \n");
    ConfigManager mgr(path);
    mgr.load();
    EXPECT_EQ(mgr.apiKey("openai"), "sk-test");
}

TEST_F(ConfigManagerTest, CustomEndpoint) {
    auto path = writeConfig("custom_endpoint = http://localhost:11434/v1/chat/completions\n");
    ConfigManager mgr(path);
    mgr.load();
    EXPECT_EQ(mgr.customEndpoint(), "http://localhost:11434/v1/chat/completions");
}

TEST_F(ConfigManagerTest, EnvVarOverridesProvider) {
    auto path = writeConfig("default_provider = openai\n");
    ConfigManager mgr(path);
    mgr.load();

    setenv("HOW_PROVIDER", "anthropic", 1);
    EXPECT_EQ(mgr.provider(), "anthropic");
    unsetenv("HOW_PROVIDER");
}

TEST_F(ConfigManagerTest, EnvVarOverridesModel) {
    auto path = writeConfig("openai_model = gpt-4o-mini\n");
    ConfigManager mgr(path);
    mgr.load();

    setenv("HOW_MODEL", "gpt-5-nano", 1);
    EXPECT_EQ(mgr.model("openai"), "gpt-5-nano");
    unsetenv("HOW_MODEL");
}
