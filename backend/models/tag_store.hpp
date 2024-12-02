#ifndef MODELS_TAG_STORE_HPP
#define MODELS_TAG_STORE_HPP

#include <atomic>
#include <nlohmann/json.hpp>
#include <string>
#include <tbb/concurrent_hash_map.h>
#include <thread>

// 定义 Tag 结构体
struct Tag
{
    int id;
    std::string name;

    nlohmann::json to_json() const
    {
        return {{"id", id}, {"name", name}};
    }

    static Tag from_json(const nlohmann::json &j)
    {
        return {j["id"].get<int>(), j["name"].get<std::string>()};
    }
};

// 定义 TagStore 类
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

    void load_from_file();
};

extern TagStore tag_store;

#endif // MODELS_TAG_STORE_HPP