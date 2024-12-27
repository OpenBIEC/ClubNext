#ifndef MODELS_DRAFT_STORE_HPP
#define MODELS_DRAFT_STORE_HPP

#include <ctime>
#include <nlohmann/json.hpp>
#include <string>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>
#include <vector>

using json = nlohmann::json;

struct Draft
{
    int draft_id;
    std::string author;
    std::string content;
    std::vector<std::string> media_paths;
    std::time_t timestamp;

    json to_json() const
    {
        return {{"draft_id", draft_id},
                {"author", author},
                {"content", content},
                {"media_paths", media_paths},
                {"timestamp", timestamp}};
    }

    static Draft from_json(const json &j)
    {
        return {j["draft_id"].get<int>(), j["author"].get<std::string>(), j["content"].get<std::string>(),
                j["media_paths"].get<std::vector<std::string>>(), j["timestamp"].get<std::time_t>()};
    }
};

class DraftStore
{
  public:
    using VectorType = tbb::concurrent_vector<Draft>;
    using MapType = tbb::concurrent_hash_map<std::string, VectorType>;

    DraftStore();
    ~DraftStore();

    bool add_draft(const std::string &username, const Draft &draft);
    bool edit_draft(const std::string &username, int draft_id, const std::string &new_content);
    bool delete_draft(const std::string &username, int draft_id);
    bool add_media_to_draft(const std::string &username, int draft_id, const std::string &media_path);
    std::vector<Draft> get_drafts(const std::string &username);

    void save_to_file();
    void load_from_file();

  private:
    MapType drafts;
    std::string file_path;

    std::string get_draft_directory(const std::string &username);
    std::string get_draft_media_path(int draft_id);
};

extern DraftStore draft_store;

#endif // MODELS_DRAFT_STORE_HPP
