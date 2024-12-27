#include "models/tag_store.hpp"
#include "config.hpp"
#include <fstream>
#include <iostream>

TagStore::TagStore(const std::string &file_path) : file_path(file_path), stop_saving(false)
{
    load_from_file();
    save_thread = std::thread([this]() {
        while (!stop_saving.load())
        {
            std::this_thread::sleep_for(std::chrono::seconds(config.PERIODIC_SAVE_DURATION));
            save_to_file();
        }
    });
}

TagStore::~TagStore()
{
    stop_saving.store(true);
    if (save_thread.joinable())
    {
        save_thread.join();
    }
    save_to_file();
}

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

void TagStore::save_to_file()
{
    if (is_saving.test_and_set())
    {
        return;
    }

    try
    {
        json json_data = json::array();
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

void TagStore::load_from_file()
{
    try
    {
        std::ifstream file(file_path);
        if (file.is_open())
        {
            json json_data;
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

TagStore::MapType &TagStore::get_tags()
{
    return tags;
}
