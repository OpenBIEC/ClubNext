#include "models/authenticate.hpp"

// 身份验证函数实现
bool authenticate_user(const httplib::Request &req, std::string &username)
{
    auto auth_header = req.get_header_value("Authorization");
    if (!auth_header.empty() && auth_header.rfind("Bearer ", 0) == 0)
    {
        std::string token = auth_header.substr(7);
        return session_store.validate_session(token, username);
    }
    return false;
}
