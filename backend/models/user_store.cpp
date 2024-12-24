#include "models/user_store.hpp"
#include "config.hpp"
#include <algorithm>
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
            std::this_thread::sleep_for(std::chrono::seconds(config.PERIODIC_SAVE));
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

bool UserStore::accept_mode(const std::string &username, int mode)
{
    MapType::accessor acc;
    if (users.find(acc, username))
    {
        return acc->second.user_mode > mode;
    }
    return false;
}

bool UserStore::store_active_code(const std::string &username, const std::string code)
{
    MapType::accessor acc;
    if (users.find(acc, username))
    {
        acc->second.active_code = code;
        return true;
    }
    return false;
}

bool UserStore::active_user(const std::string &username, const std::string code)
{
    MapType::accessor acc;
    if (users.find(acc, username))
    {
        if (acc->second.user_mode > 0 || acc->second.active_code.empty())
        {
            return false;
        }
        if (acc->second.active_code == code)
        {
            acc->second.user_mode = config.DEFAULT_USER_MODE;
            return true;
        }
        return false;
    }
    return false;
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

bool UserStore::use_space(const std::string &username, size_t file_size)
{
    MapType::accessor acc;
    if (users.find(acc, username))
    {
        if (acc->second.user_space.load() > file_size)
        {
            acc->second.user_space -= file_size;
            return true;
        }
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

void UserStore::follower_user(const std::string &username, const std::string &user_id)
{
    MapType::accessor acc;
    if (users.find(acc, username))
    {
        acc->second.followers++;
        acc->second.follower_names.push_back(user_id);
    }
}

void UserStore::following_user(const std::string &username, const std::string &user_id)
{
    MapType::accessor acc;
    if (users.find(acc, username))
    {
        acc->second.followings++;
        acc->second.following_names.push_back(user_id);
    }
}

void UserStore::unfollower_user(const std::string &username, const std::string &user_id)
{
    MapType::accessor acc;
    if (users.find(acc, username))
    {
        acc->second.followers--;
        auto item = std::ranges::find(acc->second.follower_names, username);
        if (item != acc->second.follower_names.end())
            *item = "";
    }
}

void UserStore::unfollowing_user(const std::string &username, const std::string &user_id)
{
    MapType::accessor acc;
    if (users.find(acc, username))
    {
        acc->second.followings--;
        auto item = std::ranges::find(acc->second.following_names, username);
        if (item != acc->second.following_names.end())
            *item = "";
    }
}

void UserStore::save_to_file()
{
    if (is_saving.test_and_set())
    {
        return;
    }

    try
    {
        json json_data = json::array();
        for (const auto &item : users)
        {
            if (item.second.user_mode == 0)
            {
                continue;
            }
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
            json json_data;
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
