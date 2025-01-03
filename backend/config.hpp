#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstddef>
#include <string>

struct Config
{
    const std::string BASE_URL = "http://example.com";

    const std::string USER_STORE = "user_store.json";
    const std::string POST_STORE = "post_store.json";
    const std::string SESSION_STORE = "session_store.json";
    const std::string TAG_STORE = "tag_store.json";
    const std::string MESSAGE_STORE = "message_store.json";
    const std::string LOG_STORE = "log_store.json";
    const std::string DRAFT_STORE = "draft_store.json";
    const std::string COMMENT_STORE = "comment_store.json";

    const std::string STATIC_DIR = "./../../../../frontend/";
    const std::string USER_DIR = "./users/";
    const std::string POST_DIR = "./posts/";
    const std::string COMMENT_DIR = "COMMENT_DIR";

    const std::string VERIFY_EMAIL = "tabverify@163.com";
    const std::string VERIFY_PASSWORD = "SJc52AqkmnR4JRxh";
    const std::string VERIFY_EMAIL_SERVER = "smtps://smtp.163.com:465";

    const int DEFAULT_USER_MODE = 1;

    const size_t MAX_MESSAGE_LENGTH = 1024;               // 1KB
    const size_t MAX_USER_SPACE = 1 * 1024 * 1024 * 1024; // 1GB

    const size_t PERIODIC_SAVE_DURATION = 60;
}; // namespace Config

extern Config config;

#endif // CONFIG_HPP
