#include "user.hpp"
#include <nlohmann/json.hpp>
#include <ormpp/dbng.hpp>
#include <ormpp/mysql.hpp>

using json = nlohmann::json;
using ormpp::dbng;
using ormpp::mysql;

// 数据库表模型定义
struct user
{
    int id;
    std::string username;
    std::string password;
    std::string bio;
    std::string avatar_url;
    int followers;
    int followings;
    std::string joined_at;
};

REFLECTION(user, id, username, password, bio, avatar_url, followers, followings, joined_at)

// 注册用户
void handle_user_register(const httplib::Request &req, httplib::Response &res)
{
    try
    {
        auto db = dbng<mysql>();
        if (!db.connect("localhost", "root", "password", "forum_db"))
        {
            res.status = 500;
            res.set_content(R"({"error":"Database connection failed"})", "application/json");
            return;
        }

        auto body = json::parse(req.body);
        std::string username = body["username"];
        std::string password = body["password"];
        std::string bio = body["bio"];

        user new_user = {0, username, password, bio, "", 0, 0, ""};
        if (db.insert(new_user))
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
    catch (const std::exception &e)
    {
        res.status = 500;
        res.set_content(R"({"error":"Internal server error"})", "application/json");
    }
}

// 用户登录
void handle_user_login(const httplib::Request &req, httplib::Response &res)
{
    try
    {
        auto db = dbng<mysql>();
        if (!db.connect("localhost", "root", "password", "forum_db"))
        {
            res.status = 500;
            res.set_content(R"({"error":"Database connection failed"})", "application/json");
            return;
        }

        auto body = json::parse(req.body);
        std::string username = body["username"];
        std::string password = body["password"];

        auto result = db.query<user>("where username = ? and password = ?", username, password);
        if (!result.empty())
        {
            res.status = 200;
            res.set_content(R"({"message":"Login successful"})", "application/json");
        }
        else
        {
            res.status = 401;
            res.set_content(R"({"error":"Invalid credentials"})", "application/json");
        }
    }
    catch (const std::exception &e)
    {
        res.status = 500;
        res.set_content(R"({"error":"Internal server error"})", "application/json");
    }
}

// 获取当前用户信息
void handle_user_profile(const httplib::Request &req, httplib::Response &res)
{
    try
    {
        auto db = dbng<mysql>();
        if (!db.connect("localhost", "root", "password", "forum_db"))
        {
            res.status = 500;
            res.set_content(R"({"error":"Database connection failed"})", "application/json");
            return;
        }

        int user_id = 1; // 示例：当前用户ID从会话中获取
        auto result = db.query<user>("where id = ?", user_id);
        if (!result.empty())
        {
            json response = result[0];
            res.status = 200;
            res.set_content(response.dump(), "application/json");
        }
        else
        {
            res.status = 404;
            res.set_content(R"({"error":"User not found"})", "application/json");
        }
    }
    catch (const std::exception &e)
    {
        res.status = 500;
        res.set_content(R"({"error":"Internal server error"})", "application/json");
    }
}

// 上传/更新头像
void handle_user_update_avatar(const httplib::Request &req, httplib::Response &res)
{
    res.set_content(R"({"message":"Avatar updated successfully"})", "application/json");
}

// 获取陌生用户资料
void handle_user_get_profile(const httplib::Request &req, httplib::Response &res)
{
    try
    {
        auto db = dbng<mysql>();
        if (!db.connect("localhost", "root", "password", "forum_db"))
        {
            res.status = 500;
            res.set_content(R"({"error":"Database connection failed"})", "application/json");
            return;
        }

        int user_id = std::stoi(req.matches[1]);
        auto result = db.query<user>("where id = ?", user_id);
        if (!result.empty())
        {
            json response = result[0];
            res.status = 200;
            res.set_content(response.dump(), "application/json");
        }
        else
        {
            res.status = 404;
            res.set_content(R"({"error":"User not found"})", "application/json");
        }
    }
    catch (const std::exception &e)
    {
        res.status = 500;
        res.set_content(R"({"error":"Internal server error"})", "application/json");
    }
}
