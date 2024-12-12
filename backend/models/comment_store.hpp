#ifndef MODELS_COMMENT_STORE_HPP
#define MODELS_COMMENT_STORE_HPP

#include <ctime>
#include <nlohmann/json.hpp>
#include <string>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>
#include <vector>

using json = nlohmann::json;

struct Comment
{
    int comment_id;
    std::string author;
    std::string content_path;
    std::time_t timestamp;
    bool is_deleted;

    json to_json() const
    {
        return {{"comment_id", comment_id},
                {"author", author},
                {"content_path", content_path},
                {"timestamp", timestamp},
                {"is_deleted", is_deleted}};
    }

    static Comment from_json(const json &j)
    {
        return {j["comment_id"].get<int>(), j["author"].get<std::string>(), j["content_path"].get<std::string>(),
                j["timestamp"].get<std::time_t>(), j["is_deleted"].get<bool>()};
    }
};

class CommentStore
{
  public:
    using VectorType = tbb::concurrent_vector<Comment>;
    using MapType = tbb::concurrent_hash_map<int, VectorType>;

    CommentStore();
    ~CommentStore();

    bool add_comment(int post_id, const Comment &comment);
    bool edit_comment(int post_id, int comment_id, const std::string &new_content);
    bool delete_comment(int post_id, int comment_id);
    bool get_comment(int post_id, int comment_id, Comment &comment);
    std::vector<Comment> get_comments(int post_id);

    void save_to_file();
    void load_from_file();

  private:
    MapType comments_map;

    std::string get_post_comment_file(int post_id);
    std::string get_comment_directory(int post_id, int comment_id);
};

extern CommentStore comment_store;

#endif // MODELS_COMMENT_STORE_HPP
