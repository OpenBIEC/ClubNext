#include "routes/comment.hpp"
#include "config.hpp"
#include "models/authenticate.hpp"
#include "models/comment_store.hpp"
#include "models/log_store.hpp"
#include "models/user_store.hpp"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>


namespace fs = std::filesystem;

void handle_get_comments(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing get comments request");
    int post_id = std::stoi(req.matches[1]);
    auto comments = comment_store.get_comments(post_id);

    json response = {{"post_id", post_id}, {"comments", json::array()}};
    for (const auto &comment : comments)
    {
        std::ifstream file(comment.content_path);
        if (!file.is_open())
        {
            log_store.add_log(LogLevel::WARNING_LOG, "Failed to open comment file: " + comment.content_path);
            continue;
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        json comment_json = comment.to_json();
        comment_json["content"] = content;
        response["comments"].push_back(comment_json);
    }

    log_store.add_log(LogLevel::INFO_LOG, "Comments retrieved successfully for post ID: " + std::to_string(post_id));
    res.set_content(response.dump(), "application/json");
}

void handle_post_comment(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing post comment request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized post comment request");
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
        log_store.add_log(LogLevel::WARNING_LOG, "Invalid JSON in post comment request");
        res.status = 400;
        res.set_content("{\"message\":\"Invalid JSON\"}", "application/json");
        return;
    }

    int post_id = std::stoi(req.matches[1]);
    Comment comment = {static_cast<int>(std::time(nullptr)), username,
                       config.COMMENT_DIR + "/post_" + std::to_string(post_id) + "/comment_" +
                           std::to_string(static_cast<int>(std::time(nullptr))) + "/content.md",
                       std::time(nullptr), false};

    fs::create_directories(fs::path(comment.content_path).parent_path());
    std::ofstream file(comment.content_path);
    if (!file.is_open())
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Failed to save comment: " + comment.content_path);
        res.status = 500;
        res.set_content("{\"message\":\"Failed to save comment\"}", "application/json");
        return;
    }
    file << body["content"].get<std::string>();
    file.close();

    comment_store.add_comment(post_id, comment);

    log_store.add_log(LogLevel::INFO_LOG, "Comment posted successfully with ID: " + std::to_string(comment.comment_id));
    res.set_content(
        "{\"message\":\"Comment posted successfully\", \"comment_id\":" + std::to_string(comment.comment_id) + "}",
        "application/json");
}

void handle_edit_comment(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing edit comment request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized edit comment request");
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    int post_id = std::stoi(req.matches[1]);
    int comment_id = std::stoi(req.matches[2]);
    Comment comment;

    if (!comment_store.get_comment(post_id, comment_id, comment))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Comment not found for edit request: " + std::to_string(comment_id));
        res.status = 404;
        res.set_content("{\"error\":\"Comment not found\"}", "application/json");
        return;
    }

    if (comment.author != username)
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Forbidden edit comment request by user: " + username);
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
        log_store.add_log(LogLevel::WARNING_LOG, "Invalid JSON in edit comment request");
        res.status = 400;
        res.set_content("{\"message\":\"Invalid JSON\"}", "application/json");
        return;
    }

    if (comment_store.edit_comment(post_id, comment_id, body["content"].get<std::string>()))
    {
        log_store.add_log(LogLevel::INFO_LOG, "Comment edited successfully with ID: " + std::to_string(comment_id));
        res.set_content("{\"message\":\"Comment updated successfully\"}", "application/json");
    }
    else
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Failed to edit comment with ID: " + std::to_string(comment_id));
        res.status = 404;
        res.set_content("{\"message\":\"Comment not found\"}", "application/json");
    }
}

void handle_delete_comment(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing delete comment request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized delete comment request");
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    int post_id = std::stoi(req.matches[1]);
    int comment_id = std::stoi(req.matches[2]);
    Comment comment;

    if (!comment_store.get_comment(post_id, comment_id, comment))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Comment not found for delete request: " + std::to_string(comment_id));
        res.status = 404;
        res.set_content("{\"error\":\"Comment not found\"}", "application/json");
        return;
    }

    if (comment.author != username)
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Forbidden delete comment request by user: " + username);
        res.status = 403;
        res.set_content("{\"error\":\"Forbidden\"}", "application/json");
        return;
    }

    if (comment_store.delete_comment(post_id, comment_id))
    {
        log_store.add_log(LogLevel::INFO_LOG, "Comment deleted successfully with ID: " + std::to_string(comment_id));
        res.set_content("{\"message\":\"Comment deleted successfully\"}", "application/json");
    }
    else
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Failed to delete comment with ID: " + std::to_string(comment_id));
        res.status = 404;
        res.set_content("{\"message\":\"Comment not found\"}", "application/json");
    }
}

void handle_upload_comment_media(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing upload comment media request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized upload comment media request");
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    int post_id = std::stoi(req.matches[1]);
    int comment_id = std::stoi(req.matches[2]);
    Comment comment;

    if (!comment_store.get_comment(post_id, comment_id, comment))
    {
        log_store.add_log(LogLevel::WARNING_LOG,
                          "Comment not found for upload media request: " + std::to_string(comment_id));
        res.status = 404;
        res.set_content("{\"error\":\"Comment not found\"}", "application/json");
        return;
    }

    if (comment.author != username)
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Forbidden upload comment media request by user: " + username);
        res.status = 403;
        res.set_content("{\"error\":\"Forbidden\"}", "application/json");
        return;
    }

    auto file = req.get_file_value("media");
    if (file.filename.empty())
    {
        log_store.add_log(LogLevel::WARNING_LOG, "No file uploaded in upload comment media request");
        res.status = 400;
        res.set_content(R"({\"error\":\"No file uploaded\"})", "application/json");
        return;
    }

    if (!user_store.use_space(username, file.content.size()))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "File size exceeds limit for user: " + username);
        res.status = 400;
        res.set_content(R"({\"error\":\"File size exceeds limit of 20MB\"})", "application/json");
        return;
    }

    std::string media_path = config.COMMENT_DIR + "/post_" + std::to_string(post_id) + "/comment_" +
                             std::to_string(comment_id) + "/" + file.filename;

    fs::create_directories(fs::path(media_path).parent_path());
    std::ofstream ofs(media_path, std::ios::binary);
    if (!ofs.is_open())
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Failed to save comment media: " + media_path);
        res.status = 500;
        res.set_content(R"({\"error\":\"Failed to save media file\"})", "application/json");
        return;
    }
    ofs.write(file.content.data(), file.content.size());
    ofs.close();

    log_store.add_log(LogLevel::INFO_LOG, "Media uploaded successfully for comment ID: " + std::to_string(comment_id));
    res.set_content("{\"message\":\"Media uploaded successfully\"}", "application/json");
}
