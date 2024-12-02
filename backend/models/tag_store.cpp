#include "models/tag_store.hpp"
#include <chrono>
#include <fstream>
#include <iostream>

// 构造函数
TagStore::TagStore(const std::string &file_path) : file_path(file_path), stop_saving(false)
{
    load_from_file();
    save_thread = std::thread([this]() {
        while (!stop_saving.load())
        {
            std::this_thread::sleep_for(std::chrono::minutes(1));
            save_to_file();
        }
    });
}

// 析构函数
TagStore::~TagStore()
{
    stop_saving.store(true);
    if (save_thread.joinable())
    {
        save_thread.join();
    }
    save_to_file();
}

// 添加标签
bool TagStore::add_tag(const Tag &tag)
{
    MapType::accessor acc;
    if (tags.insert(acc, tag.id))
    {
        acc->second = tag;
        return true;
    }
    return false;
}

// 获取标签
bool TagStore::get_tag(int id, Tag &tag)
{
    MapType::const_accessor acc;
    if (tags.find(acc, id))
    {
        tag = acc->second;
        return true;
    }
    return false;
}

// 保存到文件
void TagStore::save_to_file()
{
    try
    {
        nlohmann::json json_data = nlohmann::json::array();
        for (const auto &item : tags)
        {
            json_data.push_back(item.second.to_json());
        }
        std::ofstream file(file_path, std::ios::out | std::ios::trunc);
        file << json_data.dump(4);
        file.close();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error saving tag store to file: " << e.what() << std::endl;
    }
}

// 从文件加载
void TagStore::load_from_file()
{
    try
    {
        std::ifstream file(file_path);
        if (file.is_open())
        {
            nlohmann::json json_data;
            file >> json_data;
            for (const auto &item : json_data)
            {
                Tag tag = Tag::from_json(item);
                add_tag(tag);
            }
            file.close();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading tag store from file: " << e.what() << std::endl;
    }
}

// 获取 tags 的引用
TagStore::MapType &TagStore::get_tags()
{
    return tags;
}
