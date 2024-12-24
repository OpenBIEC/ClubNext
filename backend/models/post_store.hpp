#ifndef MODELS_POST_STORE_HPP
#define MODELS_POST_STORE_HPP

#include <algorithm>
#include <atomic>
#include <ctime>
#include <iterator>
#include <nlohmann/json.hpp>
#include <ranges>
#include <string>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>
#include <thread>
#include <vector>

using json = nlohmann::json;

struct Post
{
    using VectorType = tbb::concurrent_vector<std::string>;

    int id;
    std::string author;
    std::string content;
    std::vector<std::string> media;
    std::vector<int> tags;
    std::atomic<int> like_count;
    VectorType liked_by_users;
    std::atomic<int> comment_count;
    std::time_t created_at;

    Post() = default;
    Post(int postId, std::string authorId, const std::string &postContent)
        : id(postId), author(authorId), content(postContent), like_count(0), comment_count(0)
    {
        created_at = std::time(nullptr);
    }

    Post(const Post &other)
        : id(other.id), author(other.author), content(other.content), media(other.media), tags(other.tags),
          like_count(other.like_count.load()), liked_by_users(other.liked_by_users),
          comment_count(other.comment_count.load()), created_at(other.created_at)
    {
    }

    Post &operator=(const Post &other)
    {
        if (this == &other)
            return *this;

        id = other.id;
        author = other.author;
        content = other.content;
        media = other.media;
        tags = other.tags;

        like_count.store(other.like_count.load());
        comment_count.store(other.comment_count.load());

        liked_by_users = other.liked_by_users;

        created_at = other.created_at;

        return *this;
    }

    json to_json() const
    {
        std::vector<std::string> liked_by_users_vector;
        std::ranges::copy(liked_by_users | std::ranges::views::filter([](std::string str) { return !str.empty(); }),
                          std::back_inserter(liked_by_users_vector));

        return {{"id", id},
                {"author", author},
                {"content", content},
                {"media", media},
                {"tags", tags},
                {"like_count", like_count.load()},
                {"liked_by_users", liked_by_users_vector},
                {"comment_count", comment_count.load()},
                {"created_at", created_at}};
    }

    static Post from_json(const json &j)
    {
        Post post(j["id"].get<int>(), j["author"].get<std::string>(), j["content"].get<std::string>());
        post.media = j["media"].get<std::vector<std::string>>();
        post.tags = j["tags"].get<std::vector<int>>();
        post.like_count.store(j["like_count"].get<int>());
        post.comment_count.store(j["comment_count"].get<int>());
        post.created_at = j["created_at"].get<std::time_t>();

        for (auto &item : j["liked_by_users"].get<std::vector<std::string>>())
        {
            post.liked_by_users.push_back(item);
        }

        return post;
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
    bool set_post(int id, const Post &post);
    bool get_acc(int id, MapType::accessor &acc);
    void save_to_file();

    MapType &get_posts();

  private:
    MapType posts;
    std::string file_path;
    std::atomic<bool> stop_saving;
    std::thread save_thread;
    std::atomic_flag is_saving = ATOMIC_FLAG_INIT;

    void load_from_file();
};

extern PostStore post_store;

#endif // MODELS_POST_STORE_HPP
