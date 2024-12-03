#include "models/authenticate.hpp"
#include "models/session_store.hpp"
#include "models/user_store.hpp"

bool authenticate_user(const httplib::Request &req, std::string &username)
{
    if (!user_store.is_active_user(username))
    {
        return false;
    }
    auto auth_header = req.get_header_value("Authorization");
    if (!auth_header.empty() && auth_header.rfind("Bearer ", 0) == 0)
    {
        std::string token = auth_header.substr(7);
        return session_store.validate_session(token, username);
    }
    return false;
}
