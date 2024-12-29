#ifndef MODELS_DRAFT_STORE_HPP
#define MODELS_DRAFT_STORE_HPP

#include <atomic>
#include <ctime>
#include <nlohmann/json.hpp>
#include <string>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>
#include <vector>

using json = nlohmann::json;

struct Draft
{
    int id;
    std::string author;
    std::string content;
    std::vector<std::string> media;
    std::time_t timestamp;

    Draft() = default;
    Draft(int draftId, std::string authorId, const std::string &draftContent)
        : id(draftId), author(authorId), content(draftContent), timestamp(std::time(nullptr))
    {
    }

    Draft(const Draft &other)
        : id(other.id), author(other.author), content(other.content), media(other.media), timestamp(other.timestamp)
    {
    }

    Draft &operator=(const Draft &other)
    {
        if (this == &other)
            return *this;

        id = other.id;
        author = other.author;
        content = other.content;
        media = other.media;
        timestamp = other.timestamp;

        return *this;
    }

    json to_json() const
    {
        return {{"id", id}, {"author", author}, {"content", content}, {"media", media}, {"timestamp", timestamp}};
    }

    static Draft from_json(const json &j)
    {
        Draft draft;
        draft.id = j["id"].get<int>();
        draft.author = j["author"].get<std::string>();
        draft.content = j["content"].get<std::string>();
        draft.media = j["media"].get<std::vector<std::string>>();
        draft.timestamp = j["timestamp"].get<std::time_t>();
        return draft;
    }
};

class DraftStore
{
  public:
    using DraftMapType = tbb::concurrent_hash_map<int, Draft>;
    using UserMapType = tbb::concurrent_hash_map<std::string, DraftMapType>;

    DraftStore(const std::string &file_path);
    ~DraftStore();

    bool add_draft(const std::string &username, const Draft &draft);
    bool get_draft(const std::string &username, int id, Draft &draft);
    bool set_draft(const std::string &username, int id, const Draft &draft);
    bool delete_draft(const std::string &username, int id);
    void save_to_file();

    UserMapType &get_drafts();

    std::atomic<size_t> draft_count{0};

  private:
    UserMapType drafts;
    std::string file_path;
    std::atomic<bool> stop_saving;
    std::thread save_thread;
    std::atomic_flag is_saving = ATOMIC_FLAG_INIT;

    void load_from_file();
};

extern DraftStore draft_store;

#endif // MODELS_DRAFT_STORE_HPP
