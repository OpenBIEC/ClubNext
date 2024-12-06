#include "routes/user.hpp"
#include "config.hpp"
#include "models/authenticate.hpp"
#include "models/send_verification_code.hpp"
#include "models/session_store.hpp"
#include "models/user_store.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void handle_user_register(const httplib::Request &req, httplib::Response &res)
{
    try
    {
        json body = json::parse(req.body);

        User user = {body["username"].get<std::string>(),
                     body["password"].get<std::string>(),
                     body["email"].get<std::string>(),
                     "",
                     "",
                     User::VectorType(),
                     User::VectorType(),
                     0,
                     0,
                     std::time(nullptr),
                     false};

        if (user_store.register_user(user))
        {
            res.status = 200;
            res.set_content(R"({"message":"User registered successfully"})", "application/json");
        }
        else
        {
            res.status = 400;
            res.set_content(R"({"error":"Username already exists"})", "application/json");
        }
    }
    catch (...)
    {
        res.status = 400;
        res.set_content(R"({"error":"Invalid JSON"})", "application/json");
    }
}

void handle_user_login(const httplib::Request &req, httplib::Response &res)
{
    try
    {
        nlohmann::json body = nlohmann::json::parse(req.body);

        std::string username = body["username"].get<std::string>();
        std::string password = body["password"].get<std::string>();

        if (user_store.login_user(username, password))
        {
            session_store.remove_session(username);

            std::string token = session_store.create_session(username);

            res.status = 200;
            res.set_header("Set-Cookie", "sessionid=" + token + "; Path=/; HttpOnly; Secure");
            res.set_content(nlohmann::json({{"message", "Login successful"}, {"token", token}}).dump(),
                            "application/json");
        }
        else
        {
            res.status = 401;
            res.set_content(R"({"error":"Invalid credentials"})", "application/json");
        }
    }
    catch (...)
    {
        res.status = 400;
        res.set_content(R"({"error":"Invalid JSON"})", "application/json");
    }
}

void handle_user_profile(const httplib::Request &req, httplib::Response &res)
{
    std::string username;
    if (!authenticate_user(req, username))
    {
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return;
    }

    User user;
    if (user_store.get_user(username, user))
    {
        nlohmann::json response = {{"username", user.username},
                                   {"bio", user.bio},
                                   {"avatar", user.avatar_url},
                                   {"followers", static_cast<int>(user.followers)},
                                   {"followings", static_cast<int>(user.followings)},
                                   {"joined_at", user.joined_at}};
        res.status = 200;
        res.set_content(response.dump(), "application/json");
    }
    else
    {
        res.status = 404;
        res.set_content(R"({"error":"User not found"})", "application/json");
    }
}

void handle_user_update_profile(const httplib::Request &req, httplib::Response &res)
{
    std::string username;
    if (!authenticate_user(req, username))
    {
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return;
    }

    try
    {
        nlohmann::json body = nlohmann::json::parse(req.body);

        std::string new_bio = body.value("bio", "");
        std::string new_avatar_url = body.value("avatar_url", "");

        User user;
        if (user_store.get_user(username, user))
        {
            user.bio = new_bio.empty() ? user.bio : new_bio;
            user.avatar_url = new_avatar_url.empty() ? user.avatar_url : new_avatar_url;
            user_store.register_user(user);

            res.status = 200;
            res.set_content(R"({"message":"Profile updated successfully"})", "application/json");
        }
        else
        {
            res.status = 404;
            res.set_content(R"({"error":"User not found"})", "application/json");
        }
    }
    catch (...)
    {
        res.status = 400;
        res.set_content(R"({"error":"Invalid JSON"})", "application/json");
    }
}

void handle_user_update_avatar(const httplib::Request &req, httplib::Response &res)
{
    std::string username;
    if (!authenticate_user(req, username))
    {
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return;
    }

    auto file = req.get_file_value("avatar");
    if (file.filename.empty())
    {
        res.status = 400;
        res.set_content(R"({"error":"No file uploaded"})", "application/json");
        return;
    }

    if (file.content.size() > Config::MAX_IMAGE_SIZE)
    {
        res.status = 400;
        res.set_content(R"({"error":"File size exceeds limit of 2MB"})", "application/json");
        return;
    }

    try
    {
        std::string content_type = req.get_header_value("Content-Type");
        std::string extension;

        if (content_type == "image/png")
        {
            extension = "png";
        }
        else if (content_type == "image/jpeg")
        {
            extension = "jpg";
        }
        else if (content_type == "image/gif")
        {
            extension = "gif";
        }
        else
        {
            res.status = 400;
            res.set_content(R"({"error":"Unsupported image type"})", "application/json");
            return;
        }

        std::string user_dir = Config::USER_DIR + username + "/";
        std::filesystem::create_directories(user_dir);

        auto now = std::time(nullptr);
        std::stringstream timestamp;
        timestamp << std::put_time(std::localtime(&now), "%Y%m%d%H%M%S");
        std::string filename = "avatar_" + timestamp.str() + "." + extension;

        std::string filepath = user_dir + filename;
        std::ofstream ofs(filepath, std::ios::binary);
        ofs.write(file.content.data(), file.content.size());
        ofs.close();

        std::string avatar_url = Config::BASE_URL + username + "/" + filename;
        user_store.update_avatar(username, avatar_url);

        nlohmann::json response = {{"message", "Avatar uploaded successfully"}, {"avatar_url", avatar_url}};
        res.status = 200;
        res.set_content(response.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        res.status = 500;
        nlohmann::json response = {{"error", "Failed to upload avatar"}, {"details", e.what()}};
        res.set_content(response.dump(), "application/json");
    }
}

void handle_user_get_profile(const httplib::Request &req, httplib::Response &res)
{
    std::string username = req.matches[1].str();
    User user;
    if (user_store.get_user(username, user))
    {
        json response = {{"username", user.username},
                         {"bio", user.bio},
                         {"avatar", user.avatar_url},
                         {"followers", user.followers.load()},
                         {"followings", user.followings.load()},
                         {"joined_at", user.joined_at}};
        res.status = 200;
        res.set_content(response.dump(), "application/json");
    }
    else
    {
        res.status = 404;
        res.set_content(R"({"error":"User not found"})", "application/json");
    }
}

void send_verify_email(const httplib::Request &req, httplib::Response &res)
{
    if (!req.has_param("username"))
    {
        res.status = 400;
        res.set_content("{\"error\":\"User parameter is required\"}", "application/json");
        return;
    }

    std::string username = req.get_param_value("username");
    std::string email = req.get_param_value("email");
    std::string code = generate_verification_code();

    if (user_store.store_active_code(username, code))
    {
        res.status = 500;
        res.set_content("{\"error\":\"Accout email already verified\"}", "application/json");
    }

    if (send_verification_code(email, code))
    {
        res.set_content("{\"message\":\"Verification code sent successfully\"}", "application/json");
    }
    else
    {
        res.status = 500;
        res.set_content("{\"error\":\"Failed to send email\"}", "application/json");
    }
}

void verify_user_email(const httplib::Request &req, httplib::Response &res)
{
    if (!req.has_param("username") || !req.has_param("code"))
    {
        res.status = 400;
        res.set_content("{\"error\":\"Username and code parameters are required\"}", "application/json");
        return;
    }

    std::string username = req.get_param_value("username");
    std::string code = req.get_param_value("code");

    if (!user_store.active_user(username, code))
    {
        res.status = 401;
        res.set_content("{\"error\":\"Invalid verification code\"}", "application/json");
        return;
    }

    res.status = 200;
    res.set_content("{\"error\":\"Verification successful\"}", "application/json");
}