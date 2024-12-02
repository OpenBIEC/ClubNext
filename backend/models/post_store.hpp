#ifndef MODELS_POST_STORE_HPP
#define MODELS_POST_STORE_HPP

#include <atomic>
#include <ctime>
#include <nlohmann/json.hpp>
#include <string>
#include <tbb/concurrent_hash_map.h>
#include <thread>
#include <vector>

// 定义 Post 结构体
struct Post
{
    int id;
    int author_id;
    std::string content;
    std::vector<std::string> media;
    std::vector<int> tags;
    int like_count;
    std::vector<int> liked_by_users;
    int comment_count;
    std::time_t created_at;

    Post() = default;
    Post(int postId, int authorId, const std::string &postContent)
        : id(postId), author_id(authorId), content(postContent), like_count(0), comment_count(0)
    {
        created_at = std::time(nullptr); // 记录创建时间
    }

    nlohmann::json to_json() const
    {
        return {{"id", id},
                {"author_id", author_id},
                {"content", content},
                {"media", media},
                {"tags", tags},
                {"like_count", like_count},
                {"liked_by_users", liked_by_users},
                {"comment_count", comment_count},
                {"created_at", created_at}};
    }

    static Post from_json(const nlohmann::json &j)
    {
        Post post(j["id"].get<int>(), j["author_id"].get<int>(), j["content"].get<std::string>());
        post.media = j["media"].get<std::vector<std::string>>();
        post.tags = j["tags"].get<std::vector<int>>();
        post.like_count = j["like_count"].get<int>();
        post.liked_by_users = j["liked_by_users"].get<std::vector<int>>();
        post.comment_count = j["comment_count"].get<int>();
        post.created_at = j["created_at"].get<std::time_t>();
        return post;
    }
};

// 定义 PostStore 类
class PostStore
{
  public:
    using MapType = tbb::concurrent_hash_map<int, Post>;

    PostStore(const std::string &file_path);
    ~PostStore();

    bool add_post(const Post &post);
    bool get_post(int id, Post &post);
    void save_to_file();

    MapType &get_posts();

  private:
    MapType posts;
    std::string file_path;
    std::atomic<bool> stop_saving;
    std::thread save_thread;

    void load_from_file();
};

extern PostStore post_store;

#endif // MODELS_POST_STORE_HPP
