#include "models/authenticate.hpp"
#include "models/session_store.hpp"
#include "models/user_store.hpp"
#include <string>

bool authenticate_user(const httplib::Request &req, std::string &username, int accept_mode)
{

    auto auth_header = req.get_header_value("Cookie");
    auto location = auth_header.find("sessionid=", 0);
    if (auth_header.empty() || location == std::string::npos)
    {
        return false;
    }
    std::string token = auth_header.substr(location + 10, location + 42);

    if (!session_store.validate_session(token, username))
    {
        return false;
    }
    if (user_store.accept_mode(username, accept_mode))
    {
        return true;
    }
    return false;
}

std::string generate_verification_code(size_t length)
{
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<size_t> distribution(0, sizeof(charset) - 2);

    std::string code;
    for (size_t i = 0; i < length; ++i)
    {
        code += charset[distribution(generator)];
    }
    return code;
}
