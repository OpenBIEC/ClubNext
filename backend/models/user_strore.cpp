#include "models/user_store.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

UserStore::UserStore(const std::string &file_path) : file_path(file_path), stop_saving(false)
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

UserStore::~UserStore()
{
    stop_saving.store(true);
    if (save_thread.joinable())
    {
        save_thread.join();
    }
    save_to_file();
}

bool UserStore::register_user(const User &user)
{
    MapType::accessor acc;
    if (users.insert(acc, user.username))
    {
        acc->second = user;
        return true;
    }
    return false;
}

bool UserStore::login_user(const std::string &username, const std::string &password)
{
    MapType::const_accessor acc;
    if (users.find(acc, username))
    {
        return acc->second.password == password;
    }
    return false;
}

bool UserStore::get_user(const std::string &username, User &user)
{
    MapType::const_accessor acc;
    if (users.find(acc, username))
    {
        user = acc->second;
        return true;
    }
    return false;
}

void UserStore::update_avatar(const std::string &username, const std::string &avatar_url)
{
    MapType::accessor acc;
    if (users.find(acc, username))
    {
        acc->second.avatar_url = avatar_url;
    }
}

void UserStore::save_to_file()
{
    try
    {
        nlohmann::json json_data = nlohmann::json::array();
        for (const auto &item : users)
        {
            json_data.push_back(item.second.to_json());
        }
        std::ofstream file(file_path, std::ios::out | std::ios::trunc);
        file << json_data.dump(4);
        file.close();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error saving user store to file: " << e.what() << std::endl;
    }
}

void UserStore::load_from_file()
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
                User user = User::from_json(item);
                register_user(user);
            }
            file.close();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading user store from file: " << e.what() << std::endl;
    }
}
