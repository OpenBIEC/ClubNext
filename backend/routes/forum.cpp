#include "routes/forum.hpp"
#include "config.hpp"
#include "models/authenticate.hpp"
#include "models/post_store.hpp"
#include "models/tag_store.hpp"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

extern PostStore post_store;
extern TagStore tag_store;

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

    std::string content = body["content"].get<std::string>();
    int post_id = ++post_id_counter;

    // 构建帖子目录和内容文件路径
    std::string post_dir = Config::POST_DIR + std::to_string(post_id) + "/";
    std::string content_file_path = post_dir + "content.md";

    // 创建帖子目录和内容文件
    if (std::filesystem::create_directories(post_dir))
    {
        std::cout << "mkdir -p " + post_dir << std::endl;
        res.status = 500;
        res.set_content("{\"error\":\"Failed to create post directory\"}", "application/json");
        return;
    }
    std::ofstream content_file(content_file_path);
    if (!content_file)
    {
        res.status = 500;
        res.set_content("{\"error\":\"Failed to create content file\"}", "application/json");
        return;
    }
    content_file << content;
    content_file.close();

    // 创建帖子元数据
    Post new_post(post_id, 1, content_file_path);
    post_store.add_post(new_post);

    res.set_content(json{{"message", "Post created successfully"}, {"post_id", post_id}}.dump(), "application/json");
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

    auto file = req.get_file_value("media");
    std::string post_dir = post.content.substr(0, post.content.find_last_of("/\\") + 1);
    std::string media_path = post_dir + file.filename;

    std::ofstream media_file(media_path, std::ios::binary);
    if (!media_file)
    {
        res.status = 500;
        res.set_content("{\"error\":\"Failed to save media file\"}", "application/json");
        return;
    }
    media_file.write(file.content.data(), file.content.size());
    media_file.close();

    post.media.push_back(media_path);
    post_store.add_post(post);

    res.set_content("{\"message\":\"Media uploaded successfully\"}", "application/json");
}

// 获取帖子详情
void get_post_detail(const httplib::Request &req, httplib::Response &res)
{
    int post_id = std::stoi(req.matches[1]);
    Post post;

    if (!post_store.get_post(post_id, post))
    {
        res.status = 404;
        res.set_content("{\"error\":\"Post not found\"}", "application/json");
        return;
    }

    res.set_content(json{{"id", post.id},
                         {"author_id", post.author_id},
                         {"content_file", post.content},
                         {"media", post.media},
                         {"tags", post.tags},
                         {"like_count", post.like_count},
                         {"liked_by_users", post.liked_by_users},
                         {"comment_count", post.comment_count},
                         {"created_at", post.created_at}}
                        .dump(),
                    "application/json");
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
        post.tags.push_back(tag_id); // 添加标签到帖子
    }

    post_store.add_post(post); // 更新帖子内容
    res.set_content("{\"message\":\"Tags added successfully\"}", "application/json");
}

// 获取推荐帖子
void get_recommend_posts(const httplib::Request &req, httplib::Response &res)
{
    std::string username;
    bool is_authenticated = authenticate_user(req, username);

    auto &posts = post_store.get_posts();
    std::vector<Post> post_list;

    // 收集所有帖子
    for (const auto &item : posts)
    {
        post_list.push_back(item.second);
    }

    // 按时间排序
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
