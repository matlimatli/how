#include "HistoryManager.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sys/stat.h>

class HistoryManagerTest : public ::testing::Test {
protected:
    std::filesystem::path tmpDir_;

    void SetUp() override {
        tmpDir_ = std::filesystem::temp_directory_path() / "how_test_history";
        std::filesystem::create_directories(tmpDir_);
    }

    void TearDown() override {
        std::filesystem::remove_all(tmpDir_);
    }

    std::string historyPath() {
        return (tmpDir_ / "history").string();
    }
};

TEST_F(HistoryManagerTest, MissingFileReturnsNullopt) {
    HistoryManager mgr(historyPath());
    auto result = mgr.loadPrevious();
    EXPECT_FALSE(result.has_value());
}

TEST_F(HistoryManagerTest, CorruptJsonReturnsNullopt) {
    auto path = historyPath();
    std::ofstream file(path);
    file << "not valid json {{{";
    file.close();

    HistoryManager mgr(path);
    auto result = mgr.loadPrevious();
    EXPECT_FALSE(result.has_value());
}

TEST_F(HistoryManagerTest, MissingFieldsReturnsNullopt) {
    auto path = historyPath();
    std::ofstream file(path);
    file << R"({"user": "hello"})";
    file.close();

    HistoryManager mgr(path);
    auto result = mgr.loadPrevious();
    EXPECT_FALSE(result.has_value());
}

TEST_F(HistoryManagerTest, SaveAndLoad) {
    auto path = historyPath();
    HistoryManager mgr(path);

    mgr.save("how to list files", "Use ls -la");
    auto result = mgr.loadPrevious();

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->userQuery, "how to list files");
    EXPECT_EQ(result->assistantReply, "Use ls -la");
}

TEST_F(HistoryManagerTest, SaveOverwritesPrevious) {
    auto path = historyPath();
    HistoryManager mgr(path);

    mgr.save("first query", "first reply");
    mgr.save("second query", "second reply");

    auto result = mgr.loadPrevious();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->userQuery, "second query");
    EXPECT_EQ(result->assistantReply, "second reply");
}

TEST_F(HistoryManagerTest, SaveEnforcesPermissions) {
    auto path = historyPath();
    HistoryManager mgr(path);

    mgr.save("query", "reply");

    struct stat st{};
    ASSERT_EQ(stat(path.c_str(), &st), 0);
    mode_t perms = st.st_mode & 0777;
    EXPECT_EQ(perms, 0600u);
}

TEST_F(HistoryManagerTest, SaveCreatesParentDirectories) {
    auto path = (tmpDir_ / "nested" / "deep" / "history").string();
    HistoryManager mgr(path);

    EXPECT_NO_THROW(mgr.save("query", "reply"));

    auto result = mgr.loadPrevious();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->userQuery, "query");
}

TEST_F(HistoryManagerTest, SaveProducesValidJson) {
    auto path = historyPath();
    HistoryManager mgr(path);

    mgr.save("query with \"quotes\" and\nnewlines", "reply <with> special & chars");

    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    // Should be valid JSON and round-trip correctly
    auto result = mgr.loadPrevious();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->userQuery, "query with \"quotes\" and\nnewlines");
    EXPECT_EQ(result->assistantReply, "reply <with> special & chars");
}
