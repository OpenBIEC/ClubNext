#include "routes/user.hpp"
#include "config.hpp"
#include "models/authenticate.hpp"
#include "models/log_store.hpp"
#include "models/send_verification_code.hpp"
#include "models/session_store.hpp"
#include "models/user_store.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void handle_user_register(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Handling user registration");
    try
    {
        json body = json::parse(req.body);

        User user = {body["username"].get<std::string>(),
                     body["password"].get<std::string>(),
                     body["email"].get<std::string>(),
                     "",
                     "https://placehold.co/150",
                     User::VectorType(),
                     User::VectorType(),
                     0,
                     0,
                     config.MAX_USER_SPACE,
                     std::time(nullptr),
                     0};

        if (user_store.register_user(user))
        {
            log_store.add_log(LogLevel::INFO_LOG, "User registered successfully: " + user.username);
            res.status = 200;
            res.set_content(R"({"message":"User registered successfully"})", "application/json");
        }
        else
        {
            log_store.add_log(LogLevel::WARNING_LOG, "Registration failed: Username already exists - " + user.username);
            res.status = 400;
            res.set_content(R"({"error":"Username already exists"})", "application/json");
        }
    }
    catch (...)
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Invalid JSON received during registration");
        res.status = 400;
        res.set_content(R"({"error":"Invalid JSON"})", "application/json");
    }
}

void handle_user_login(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Handling user login");
    try
    {
        auto body = json::parse(req.body);

        std::string username = body["username"].get<std::string>();
        std::string password = body["password"].get<std::string>();

        if (user_store.login_user(username, password))
        {
            session_store.remove_session(username);

            auto token = body.contains("remember_me") ? session_store.create_session(username)
                                                      : session_store.create_session(username, 7 * 24);

            log_store.add_log(LogLevel::INFO_LOG, "User login successful: " + username);
            res.status = 200;
            res.set_header("Set-Cookie", "sessionid=" + token + "; Path=/; HttpOnly; Secure");
            res.set_content(json({{"message", "Login successful"}, {"token", token}}).dump(), "application/json");
        }
        else
        {
            log_store.add_log(LogLevel::WARNING_LOG, "Login failed: Invalid credentials for username - " + username);
            res.status = 401;
            res.set_content(R"({"error":"Invalid credentials"})", "application/json");
        }
    }
    catch (...)
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Invalid JSON received during login");
        res.status = 400;
        res.set_content(R"({"error":"Invalid JSON"})", "application/json");
    }
}

void handle_user_profile(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Fetching user profile");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized access to user profile");
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return;
    }

    User user;
    if (user_store.get_user(username, user))
    {
        log_store.add_log(LogLevel::INFO_LOG, "User profile retrieved: " + username);
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
        log_store.add_log(LogLevel::WARNING_LOG, "User not found: " + username);
        res.status = 404;
        res.set_content(R"({"error":"User not found"})", "application/json");
    }
}

void handle_user_update_profile(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Updating user profile");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized access to update profile");
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return;
    }

    try
    {
        auto body = json::parse(req.body);

        std::string new_bio = body.value("bio", "");
        std::string new_avatar_url = body.value("avatar_url", "");

        User user;
        if (user_store.get_user(username, user))
        {
            user.bio = new_bio.empty() ? user.bio : new_bio;
            user.avatar_url = new_avatar_url.empty() ? user.avatar_url : new_avatar_url;
            user_store.register_user(user);

            log_store.add_log(LogLevel::INFO_LOG, "User profile updated successfully: " + username);
            res.status = 200;
            res.set_content(R"({"message":"Profile updated successfully"})", "application/json");
        }
        else
        {
            log_store.add_log(LogLevel::WARNING_LOG, "Profile update failed: User not found - " + username);
            res.status = 404;
            res.set_content(R"({"error":"User not found"})", "application/json");
        }
    }
    catch (...)
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Invalid JSON received during profile update");
        res.status = 400;
        res.set_content(R"({"error":"Invalid JSON"})", "application/json");
    }
}

void handle_user_update_avatar(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Handling user avatar update");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized access to update avatar");
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return;
    }

    auto file = req.get_file_value("avatar");
    if (file.filename.empty())
    {
        log_store.add_log(LogLevel::WARNING_LOG, "No file uploaded for avatar update");
        res.status = 400;
        res.set_content(R"({"error":"No file uploaded"})", "application/json");
        return;
    }

    if (!user_store.use_space(username, file.content.size()))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "User file space limit exceeded: " + username);
        res.status = 400;
        res.set_content(R"({"error":"User file space exceeds limit of 1GB"})", "application/json");
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
            log_store.add_log(LogLevel::WARNING_LOG, "Unsupported image type for avatar update");
            res.status = 400;
            res.set_content(R"({"error":"Unsupported image type"})", "application/json");
            return;
        }

        std::string user_dir = config.USER_DIR + username + "/";
        std::filesystem::create_directories(user_dir);

        auto now = std::time(nullptr);
        std::stringstream timestamp;
        timestamp << std::put_time(std::localtime(&now), "%Y%m%d%H%M%S");
        std::string filename = "avatar_" + timestamp.str() + "." + extension;

        std::string filepath = user_dir + filename;
        std::ofstream ofs(filepath, std::ios::binary);
        ofs.write(file.content.data(), file.content.size());
        ofs.close();

        std::string avatar_url = config.BASE_URL + username + "/" + filename;
        user_store.update_avatar(username, avatar_url);

        log_store.add_log(LogLevel::INFO_LOG, "Avatar updated successfully for user: " + username);
        json response = {{"message", "Avatar uploaded successfully"}, {"avatar_url", avatar_url}};
        res.status = 200;
        res.set_content(response.dump(), "application/json");
    }
    catch (const std::exception &e)
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Exception during avatar update for user: " + username);
        res.status = 500;
        json response = {{"error", "Failed to upload avatar"}, {"details", e.what()}};
        res.set_content(response.dump(), "application/json");
    }
}

void handle_user_get_profile(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Fetching profile for requested user");
    std::string username = req.matches[1].str();
    User user;
    if (user_store.get_user(username, user))
    {
        log_store.add_log(LogLevel::INFO_LOG, "User profile retrieved successfully: " + username);
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
        log_store.add_log(LogLevel::WARNING_LOG, "User not found: " + username);
        res.status = 404;
        res.set_content(R"({"error":"User not found"})", "application/json");
    }
}

void send_verify_email(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Handling email verification request");
    if (!req.has_param("username") || !req.has_param("email"))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Missing username or email in verification request");
        res.status = 400;
        res.set_content("{\"error\":\"Username and email parameter is required\"}", "application/json");
        return;
    }

    std::string username = req.get_param_value("username");
    std::string email = req.get_param_value("email");
    std::string code = generate_verification_code();

    if (user_store.accept_mode(username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Email already verified for user: " + username);
        res.status = 500;
        res.set_content("{\"error\":\"Accout email already verified\"}", "application/json");
        return;
    }

    if (!user_store.store_active_code(username, code))
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Failed to store verification code for user: " + username);
        res.status = 500;
        res.set_content("{\"error\":\"Accout not found\"}", "application/json");
        return;
    }

    if (send_verification_code(email, code))
    {
        log_store.add_log(LogLevel::INFO_LOG, "Verification code sent successfully to: " + email);
        res.set_content("{\"message\":\"Verification code sent successfully\"}", "application/json");
    }
    else
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Failed to send verification code to: " + email);
        res.status = 500;
        res.set_content("{\"error\":\"Failed to send email\"}", "application/json");
    }
}

void verify_user_email(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Handling email verification");
    if (!req.has_param("username") || !req.has_param("code"))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Missing username or code in verification request");
        res.status = 400;
        res.set_content("{\"error\":\"Username and code parameters are required\"}", "application/json");
        return;
    }

    std::string username = req.get_param_value("username");
    std::string code = req.get_param_value("code");

    if (!user_store.active_user(username, code))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Invalid verification code for user: " + username);
        res.status = 401;
        res.set_content("{\"error\":\"Invalid verification code " + code + "\"}", "application/json");
        return;
    }

    log_store.add_log(LogLevel::INFO_LOG, "Email verified successfully for user: " + username);
    res.status = 200;
    res.set_content("{\"message\":\"Verification successful\"}", "application/json");
}
