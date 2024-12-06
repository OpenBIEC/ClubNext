#include "models/authenticate.hpp"
#include "models/session_store.hpp"
#include "models/user_store.hpp"

bool authenticate_user(const httplib::Request &req, std::string &username, int accept_mode)
{
    if (!user_store.accept_mode(username, accept_mode))
    {
        return false;
    }
    auto auth_header = req.get_header_value("Cookie");
    if (!auth_header.empty() && auth_header.rfind("sessionid=", 0) == 0)
    {
        std::string token = auth_header.substr(7);
        return session_store.validate_session(token, username);
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
