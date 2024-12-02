#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

namespace Config
{
const std::string BASE_URL = "http://example.com";

const std::string USER_DIR = "./users/";
const std::string POST_DIR = "./posts/";

const size_t MAX_IMAGE_SIZE = 2 * 1024 * 1024; // 2MB
} // namespace Config

#endif // CONFIG_HPP
