#ifndef MODELS_USER_STORE_HPP
#define MODELS_USER_STORE_HPP

#include <atomic>
#include <chrono>
#include <fstream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <tbb/concurrent_hash_map.h>
#include <thread>

struct User
{
    std::string username;
    std::string password;
    std::string bio;
    std::string avatar_url;
    int followers;
    int followings;

    nlohmann::json to_json() const
    {
        return {{"username", username},     {"password", password},   {"bio", bio},
                {"avatar_url", avatar_url}, {"followers", followers}, {"followings", followings}};
    }

    static User from_json(const nlohmann::json &j)
    {
        return {j["username"].get<std::string>(),   j["password"].get<std::string>(), j["bio"].get<std::string>(),
                j["avatar_url"].get<std::string>(), j["followers"].get<int>(),        j["followings"].get<int>()};
    }
};

class UserStore
{
  public:
    using MapType = tbb::concurrent_hash_map<std::string, User>;

    UserStore(const std::string &file_path);
    ~UserStore();

    bool register_user(const User &user);
    bool login_user(const std::string &username, const std::string &password);
    bool get_user(const std::string &username, User &user);
    void update_avatar(const std::string &username, const std::string &avatar_url);

    void save_to_file();

  private:
    MapType users;
    std::string file_path;
    std::atomic<bool> stop_saving;
    std::thread save_thread;

    void load_from_file();
};

#endif // MODELS_USER_STORE_HPP
