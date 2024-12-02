#include "post_store.hpp"
#include <chrono>
#include <fstream>
#include <iostream>

PostStore::PostStore(const std::string &file_path) : file_path(file_path), stop_saving(false)
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

PostStore::~PostStore()
{
    stop_saving.store(true);
    if (save_thread.joinable())
    {
        save_thread.join();
    }
    save_to_file();
}

bool PostStore::add_post(const Post &post)
{
    MapType::accessor acc;
    if (posts.insert(acc, post.id))
    {
        acc->second = post;
        return true;
    }
    return false;
}

bool PostStore::get_post(int id, Post &post)
{
    MapType::const_accessor acc;
    if (posts.find(acc, id))
    {
        post = acc->second;
        return true;
    }
    return false;
}

void PostStore::save_to_file()
{
    try
    {
        nlohmann::json json_data = nlohmann::json::array();
        for (const auto &item : posts)
        {
            json_data.push_back(item.second.to_json());
        }
        std::ofstream file(file_path, std::ios::out | std::ios::trunc);
        file << json_data.dump(4);
        file.close();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error saving post store to file: " << e.what() << std::endl;
    }
}

void PostStore::load_from_file()
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
                Post post = Post::from_json(item);
                add_post(post);
            }
            file.close();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading post store from file: " << e.what() << std::endl;
    }
}

PostStore::MapType &PostStore::get_posts()
{
    return posts;
}
