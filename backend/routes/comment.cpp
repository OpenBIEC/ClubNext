#include "routes/comment.hpp"
#include "config.hpp"
#include "models/authenticate.hpp"
#include "models/comment_store.hpp"
#include "models/user_store.hpp"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

void handle_get_comments(const httplib::Request &req, httplib::Response &res)
{
    int post_id = std::stoi(req.matches[1]); // 假设正则捕获 post_id
    auto comments = comment_store.get_comments(post_id);

    json response = {{"post_id", post_id}, {"comments", json::array()}};
    for (const auto &comment : comments)
    {
        std::ifstream file(comment.content_path);
        if (!file.is_open())
        {
            continue; // 忽略无法读取的评论内容
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        json comment_json = comment.to_json();
        comment_json["content"] = content;
        response["comments"].push_back(comment_json);
    }

    res.set_content(response.dump(), "application/json");
}

void handle_post_comment(const httplib::Request &req, httplib::Response &res)
{
    std::string username;
    if (!authenticate_user(req, username))
    {
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    json body;
    try
    {
        body = json::parse(req.body);
    }
    catch (const std::exception &)
    {
        res.status = 400;
        res.set_content("{\"message\":\"Invalid JSON\"}", "application/json");
        return;
    }

    int post_id = std::stoi(req.matches[1]);
    Comment comment = {static_cast<int>(std::time(nullptr)), body["author"].get<std::string>(),
                       config.COMMENT_DIR + "/post_" + std::to_string(post_id) + "/comment_" +
                           std::to_string(static_cast<int>(std::time(nullptr))) + "/content.md",
                       std::time(nullptr), false};

    fs::create_directories(fs::path(comment.content_path).parent_path());
    std::ofstream file(comment.content_path);
    if (!file.is_open())
    {
        res.status = 500;
        res.set_content("{\"message\":\"Failed to save comment\"}", "application/json");
        return;
    }
    file << body["content"].get<std::string>();
    file.close();

    comment_store.add_comment(post_id, comment);

    res.set_content(
        "{\"message\":\"Comment posted successfully\", \"comment_id\":" + std::to_string(comment.comment_id) + "}",
        "application/json");
}

void handle_edit_comment(const httplib::Request &req, httplib::Response &res)
{
    std::string username;
    if (!authenticate_user(req, username))
    {
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    int post_id = std::stoi(req.matches[1]);
    int comment_id = std::stoi(req.matches[2]);
    Comment comment;

    if (!comment_store.get_comment(post_id, comment_id, comment))
    {
        res.status = 404;
        res.set_content("{\"error\":\"Comment not found\"}", "application/json");
        return;
    }

    if (comment.author != username)
    {
        res.status = 403;
        res.set_content("{\"error\":\"Forbidden\"}", "application/json");
        return;
    }

    json body;
    try
    {
        body = json::parse(req.body);
    }
    catch (const std::exception &)
    {
        res.status = 400;
        res.set_content("{\"message\":\"Invalid JSON\"}", "application/json");
        return;
    }

    if (comment_store.edit_comment(post_id, comment_id, body["content"].get<std::string>()))
    {
        res.set_content("{\"message\":\"Comment updated successfully\"}", "application/json");
    }
    else
    {
        res.status = 404;
        res.set_content("{\"message\":\"Comment not found\"}", "application/json");
    }
}

void handle_delete_comment(const httplib::Request &req, httplib::Response &res)
{
    std::string username;
    if (!authenticate_user(req, username))
    {
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    int post_id = std::stoi(req.matches[1]);
    int comment_id = std::stoi(req.matches[2]);
    Comment comment;

    if (!comment_store.get_comment(post_id, comment_id, comment))
    {
        res.status = 404;
        res.set_content("{\"error\":\"Comment not found\"}", "application/json");
        return;
    }

    if (comment.author != username)
    {
        res.status = 403;
        res.set_content("{\"error\":\"Forbidden\"}", "application/json");
        return;
    }

    if (comment_store.delete_comment(post_id, comment_id))
    {
        res.set_content("{\"message\":\"Comment deleted successfully\"}", "application/json");
    }
    else
    {
        res.status = 404;
        res.set_content("{\"message\":\"Comment not found\"}", "application/json");
    }
}

void handle_upload_comment_media(const httplib::Request &req, httplib::Response &res)
{
    std::string username;
    if (!authenticate_user(req, username))
    {
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    int post_id = std::stoi(req.matches[1]);
    int comment_id = std::stoi(req.matches[2]);
    Comment comment;

    if (!comment_store.get_comment(post_id, comment_id, comment))
    {
        res.status = 404;
        res.set_content("{\"error\":\"Comment not found\"}", "application/json");
        return;
    }

    if (comment.author != username)
    {
        res.status = 403;
        res.set_content("{\"error\":\"Forbidden\"}", "application/json");
        return;
    }

    auto file = req.get_file_value("media");
    if (file.filename.empty())
    {
        res.status = 400;
        res.set_content(R"({"error":"No file uploaded"})", "application/json");
        return;
    }

    if (!user_store.use_space(username, file.content.size()))
    {
        res.status = 400;
        res.set_content(R"({"error":"File size exceeds limit of 20MB"})", "application/json");
        return;
    }

    std::string media_path = config.COMMENT_DIR + "/post_" + std::to_string(post_id) + "/comment_" +
                             std::to_string(comment_id) + "/" + file.filename;

    fs::create_directories(fs::path(media_path).parent_path());
    std::ofstream ofs(media_path, std::ios::binary);
    ofs.write(file.content.data(), file.content.size());
    ofs.close();

    res.set_content("{\"message\":\"Media uploaded successfully\"}", "application/json");
}
