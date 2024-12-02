#include "routes/forum.hpp"
#include "models/authenticate.hpp"
#include "models/post_store.hpp"
#include "models/tag_store.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 创建帖子
void create_post(const httplib::Request &req, httplib::Response &res)
{
    std::string username;
    if (!authenticate_user(req, username))
    {
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    static int post_id_counter = 0;
    json body = json::parse(req.body, nullptr, false);
    if (body.is_discarded() || !body.contains("content"))
    {
        res.status = 400;
        res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
        return;
    }

    Post new_post(++post_id_counter, 1, body["content"].get<std::string>()); // 假设 author_id 为 1
    if (post_store.add_post(new_post))
    {
        res.set_content(json{{"message", "Post created successfully"}, {"post_id", post_id_counter}}.dump(),
                        "application/json");
    }
    else
    {
        res.status = 500;
        res.set_content("{\"error\":\"Failed to create post\"}", "application/json");
    }
}

// 上传媒体
void upload_media(const httplib::Request &req, httplib::Response &res)
{
    std::string username;
    if (!authenticate_user(req, username))
    {
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    int post_id = std::stoi(req.matches[1]);
    Post post;

    if (post_store.get_post(post_id, post))
    {
        if (post.author_id != 1)
        { // 假设 1 是验证得到的用户名 ID
            res.status = 403;
            res.set_content("{\"error\":\"Forbidden\"}", "application/json");
            return;
        }

        std::string media_url = "/path/to/media.jpg"; // 假设上传逻辑提供该路径
        post.media.push_back(media_url);
        res.set_content("{\"message\":\"Media uploaded successfully\"}", "application/json");
    }
    else
    {
        res.status = 404;
        res.set_content("{\"error\":\"Post not found\"}", "application/json");
    }
}

// 给帖子添加标签
void add_tags_to_post(const httplib::Request &req, httplib::Response &res)
{
    std::string username;
    if (!authenticate_user(req, username))
    {
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    int post_id = std::stoi(req.matches[1]);
    json body = json::parse(req.body, nullptr, false);

    if (body.is_discarded() || !body.contains("tags"))
    {
        res.status = 400;
        res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
        return;
    }

    Post post;
    if (!post_store.get_post(post_id, post))
    {
        res.status = 404;
        res.set_content("{\"error\":\"Post not found\"}", "application/json");
        return;
    }

    if (post.author_id != 1)
    { // 假设 1 是验证得到的用户名 ID
        res.status = 403;
        res.set_content("{\"error\":\"Forbidden\"}", "application/json");
        return;
    }

    for (const auto &tag_id : body["tags"])
    {
        Tag tag;
        if (!tag_store.get_tag(tag_id, tag))
        {
            res.status = 404;
            res.set_content("{\"error\":\"Tag not found\"}", "application/json");
            return;
        }
    }

    res.set_content("{\"message\":\"Tags added successfully\"}", "application/json");
}

// 获取帖子详情
void get_post_detail(const httplib::Request &req, httplib::Response &res)
{
    int post_id = std::stoi(req.matches[1]);
    Post post;

    if (post_store.get_post(post_id, post))
    {
        res.set_content(post.to_json().dump(), "application/json");
    }
    else
    {
        res.status = 404;
        res.set_content("{\"error\":\"Post not found\"}", "application/json");
    }
}

// 获取推荐帖子
void get_recommend_posts(const httplib::Request &req, httplib::Response &res)
{
    std::string username;
    bool is_authenticated = authenticate_user(req, username);

    auto &posts = post_store.get_posts();
    json response = json::array();
    int count = 0;

    // 推荐逻辑：按时间排序推荐
    for (const auto &item : posts)
    {
        if (count++ == 10)
            break;
        response.push_back({{"id", item.first}});
    }

    // 返回推荐结果
    if (is_authenticated)
    {
        res.set_content(json{{"message", "Authenticated recommendation"}, {"recommended_posts", response}}.dump(),
                        "application/json");
    }
    else
    {
        res.set_content(json{{"message", "Default recommendation"}, {"recommended_posts", response}}.dump(),
                        "application/json");
    }
}

// 获取所有标签
void get_tags(const httplib::Request &, httplib::Response &res)
{
    auto &tags = tag_store.get_tags();
    json response = json::array();

    for (const auto &item : tags)
    {
        response.push_back(item.second.to_json());
    }

    res.set_content(json{{"tags", response}}.dump(), "application/json");
}
