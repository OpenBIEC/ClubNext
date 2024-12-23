#include "routes/forum.hpp"
#include "config.hpp"
#include "models/authenticate.hpp"
#include "models/log_store.hpp"
#include "models/post_store.hpp"
#include "models/tag_store.hpp"
#include "models/user_store.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void create_post(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing create post request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized create post request");
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    static int post_id_counter = 0;
    json body = json::parse(req.body, nullptr, false);
    if (body.is_discarded() || !body.contains("content"))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Invalid JSON in create post request");
        res.status = 400;
        res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
        return;
    }

    std::string content = body["content"].get<std::string>();
    int post_id = ++post_id_counter;

    std::string post_dir = config.POST_DIR + std::to_string(post_id) + "/";
    std::string content_file_path = post_dir + "content.md";

    if (!std::filesystem::create_directories(post_dir))
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Failed to create post directory: " + post_dir);
        res.status = 500;
        res.set_content("{\"error\":\"Failed to create post directory\"}", "application/json");
        return;
    }
    std::ofstream content_file(content_file_path);
    if (!content_file)
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Failed to create content file: " + content_file_path);
        res.status = 500;
        res.set_content("{\"error\":\"Failed to create content file\"}", "application/json");
        return;
    }
    content_file << content;
    content_file.close();

    Post new_post(post_id, username, content_file_path);
    post_store.add_post(new_post);

    log_store.add_log(LogLevel::INFO_LOG, "Post created successfully with ID: " + std::to_string(post_id));
    res.set_content(json{{"message", "Post created successfully"}, {"post_id", post_id}}.dump(), "application/json");
}

void modify_post(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing modify post request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized modify post request");
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    int post_id = std::stoi(req.matches[1]);
    Post post;

    if (!post_store.get_post(post_id, post))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Post not found with ID: " + std::to_string(post_id));
        res.status = 404;
        res.set_content("{\"error\":\"Post not found\"}", "application/json");
        return;
    }

    if (post.author != username)
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Forbidden modify post request by user: " + username);
        res.status = 403;
        res.set_content("{\"error\":\"Forbidden\"}", "application/json");
        return;
    }

    json body = json::parse(req.body, nullptr, false);
    if (body.is_discarded() || !body.contains("content"))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Invalid JSON in modify post request");
        res.status = 400;
        res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
        return;
    }

    std::string content = body["content"].get<std::string>();

    std::string post_dir = config.POST_DIR + std::to_string(post_id) + "/";
    std::string content_file_path = post_dir + "content.md";

    std::ofstream content_file(content_file_path);
    if (!content_file)
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Failed to open content file: " + content_file_path);
        res.status = 500;
        res.set_content("{\"error\":\"Failed to open content file\"}", "application/json");
        return;
    }
    content_file << content;
    content_file.close();

    Post new_post(post_id, username, content_file_path);
    post_store.set_post(post_id, new_post);

    log_store.add_log(LogLevel::INFO_LOG, "Post modified successfully with ID: " + std::to_string(post_id));
    res.set_content(json{{"message", "Post modified successfully"}, {"post_id", post_id}}.dump(), "application/json");
}

void upload_media(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing upload media request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized upload media request");
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    int post_id = std::stoi(req.matches[1]);
    Post post;

    if (!post_store.get_post(post_id, post))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Post not found with ID: " + std::to_string(post_id));
        res.status = 404;
        res.set_content("{\"error\":\"Post not found\"}", "application/json");
        return;
    }

    if (post.author != username)
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Forbidden upload media request by user: " + username);
        res.status = 403;
        res.set_content("{\"error\":\"Forbidden\"}", "application/json");
        return;
    }

    auto file = req.get_file_value("media");
    if (file.filename.empty())
    {
        log_store.add_log(LogLevel::WARNING_LOG, "No file uploaded in media request");
        res.status = 400;
        res.set_content(R"({"error":"No file uploaded"})", "application/json");
        return;
    }

    if (!user_store.use_space(username, file.content.size()))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "User file space exceeded for user: " + username);
        res.status = 400;
        res.set_content(R"({"error":"User file space exceeds limit of 1GB"})", "application/json");
        return;
    }

    std::string post_dir = post.content.substr(0, post.content.find_last_of("/\\") + 1);
    std::string media_path = post_dir + file.filename;

    std::ofstream media_file(media_path, std::ios::binary);
    if (!media_file)
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Failed to save media file: " + media_path);
        res.status = 500;
        res.set_content("{\"error\":\"Failed to save media file\"}", "application/json");
        return;
    }
    media_file.write(file.content.data(), file.content.size());
    media_file.close();

    post.media.push_back(media_path);
    post_store.add_post(post);

    log_store.add_log(LogLevel::INFO_LOG, "Media uploaded successfully for post ID: " + std::to_string(post_id));
    res.set_content("{\"message\":\"Media uploaded successfully\"}", "application/json");
}

void get_post_detail(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing get post detail request");
    int post_id = std::stoi(req.matches[1]);

    Post post;
    if (!post_store.get_post(post_id, post))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Post not found with ID: " + std::to_string(post_id));
        res.status = 404;
        res.set_content("{\"error\":\"Post not found\"}", "application/json");
        return;
    }

    log_store.add_log(LogLevel::INFO_LOG, "Post detail retrieved successfully for ID: " + std::to_string(post_id));
    res.set_content(post.to_json().dump(), "application/json");
}

void add_tags_to_post(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing add tags to post request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized add tags request");
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    int post_id = std::stoi(req.matches[1]);
    json body = json::parse(req.body, nullptr, false);

    if (body.is_discarded() || !body.contains("tags"))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Invalid JSON in add tags request");
        res.status = 400;
        res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
        return;
    }

    Post post;
    if (!post_store.get_post(post_id, post))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Post not found with ID: " + std::to_string(post_id));
        res.status = 404;
        res.set_content("{\"error\":\"Post not found\"}", "application/json");
        return;
    }

    if (post.author != username)
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Forbidden add tags request by user: " + username);
        res.status = 403;
        res.set_content("{\"error\":\"Forbidden\"}", "application/json");
        return;
    }

    for (const auto &tag_id : body["tags"])
    {
        Tag tag;
        if (!tag_store.get_tag(tag_id, tag))
        {
            log_store.add_log(LogLevel::WARNING_LOG, "Tag not found with ID: " + tag_id.get<std::string>());
            res.status = 404;
            res.set_content("{\"error\":\"Tag not found\"}", "application/json");
            return;
        }
        post.tags.push_back(tag_id);
    }

    post_store.add_post(post);

    log_store.add_log(LogLevel::INFO_LOG, "Tags added successfully to post ID: " + std::to_string(post_id));
    res.set_content("{\"message\":\"Tags added successfully\"}", "application/json");
}

void get_recommend_posts(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing get recommended posts request");
    std::string username;
    bool is_authenticated = authenticate_user(req, username);

    auto &posts = post_store.get_posts();
    std::vector<Post> post_list;

    for (const auto &item : posts)
    {
        post_list.push_back(item.second);
    }

    std::sort(post_list.begin(), post_list.end(),
              [](const Post &a, const Post &b) { return a.created_at > b.created_at; });

    json response = json::array();
    int count = 0;

    for (const auto &post : post_list)
    {
        if (count++ == 10)
            break;
        response.push_back({{"id", post.id}, {"content_file", post.content}, {"created_at", post.created_at}});
    }

    if (is_authenticated)
    {
        log_store.add_log(LogLevel::INFO_LOG, "Authenticated user requesting recommendations");
        res.set_content(json{{"message", "Authenticated recommendation"}, {"recommended_posts", response}}.dump(),
                        "application/json");
    }
    else
    {
        log_store.add_log(LogLevel::INFO_LOG, "Unauthenticated user requesting recommendations");
        res.set_content(json{{"message", "Default recommendation"}, {"recommended_posts", response}}.dump(),
                        "application/json");
    }
}

void get_tags(const httplib::Request &, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing get tags request");
    auto &tags = tag_store.get_tags();
    json response = json::array();

    for (const auto &item : tags)
    {
        response.push_back(item.second.to_json());
    }

    log_store.add_log(LogLevel::INFO_LOG, "Tags retrieved successfully");
    res.set_content(json{{"tags", response}}.dump(), "application/json");
}