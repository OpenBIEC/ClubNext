#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstddef>
#include <string>

namespace Config
{
const std::string BASE_URL = "http://example.com";

const std::string USER_DIR = "./users/";
const std::string POST_DIR = "./posts/";

const std::string VERIFY_EMAIL = "tabverify@163.com";
const std::string VERIFY_PASSWORD = "SJc52AqkmnR4JRxh";
const std::string VERIFY_EMAIL_SERVER = "smtps://smtp.163.com:465";

const size_t MAX_MESSAGE_LENGTH = 1024;
const size_t MAX_IMAGE_SIZE = 2 * 1024 * 1024; // 2MB
} // namespace Config

#endif // CONFIG_HPP
