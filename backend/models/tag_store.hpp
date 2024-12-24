#ifndef MODELS_TAG_STORE_HPP
#define MODELS_TAG_STORE_HPP

#include <atomic>
#include <nlohmann/json.hpp>
#include <string>
#include <tbb/concurrent_hash_map.h>
#include <thread>

using json = nlohmann::json;

struct Tag
{
    int id;
    std::string name;

    json to_json() const
    {
        return {{"id", id}, {"name", name}};
    }

    static Tag from_json(const json &j)
    {
        return {j["id"].get<int>(), j["name"].get<std::string>()};
    }
};

class TagStore
{
  public:
    using MapType = tbb::concurrent_hash_map<int, Tag>;

    TagStore(const std::string &file_path);
    ~TagStore();

    bool add_tag(const Tag &tag);
    bool get_tag(int id, Tag &tag);
    void save_to_file();

    MapType &get_tags();

  private:
    MapType tags;
    std::string file_path;
    std::atomic<bool> stop_saving;
    std::thread save_thread;
    std::atomic_flag is_saving = ATOMIC_FLAG_INIT;

    void load_from_file();
};

extern TagStore tag_store;

#endif // MODELS_TAG_STORE_HPP
