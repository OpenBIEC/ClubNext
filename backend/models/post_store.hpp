#ifndef MODELS_POST_STORE_HPP
#define MODELS_POST_STORE_HPP

#include <atomic>
#include <nlohmann/json.hpp>
#include <string>
#include <tbb/concurrent_hash_map.h>
#include <thread>
#include <vector>

struct Post
{
    int id;
    int author_id;
    std::string content;
    std::vector<std::string> media;
    int like_count;
    std::vector<int> liked_by_users;
    int comment_count;

    nlohmann::json to_json() const
    {
        return {{"id", id},
                {"author_id", author_id},
                {"content", content},
                {"media", media},
                {"like_count", like_count},
                {"liked_by_users", liked_by_users},
                {"comment_count", comment_count}};
    }

    static Post from_json(const nlohmann::json &j)
    {
        return {j["id"].get<int>(),
                j["author_id"].get<int>(),
                j["content"].get<std::string>(),
                j["media"].get<std::vector<std::string>>(),
                j["like_count"].get<int>(),
                j["liked_by_users"].get<std::vector<int>>(),
                j["comment_count"].get<int>()};
    }
};

class PostStore
{
  public:
    using MapType = tbb::concurrent_hash_map<int, Post>;

    PostStore(const std::string &file_path);
    ~PostStore();

    bool add_post(const Post &post);
    bool get_post(int id, Post &post);
    void save_to_file();

    MapType &get_posts(); // 提供对私有成员 posts 的引用

  private:
    MapType posts;
    std::string file_path;
    std::atomic<bool> stop_saving;
    std::thread save_thread;

    void load_from_file();
};

extern PostStore post_store;

#endif // MODELS_POST_STORE_HPP
