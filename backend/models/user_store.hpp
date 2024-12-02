#ifndef MODELS_USER_STORE_HPP
#define MODELS_USER_STORE_HPP

#include <atomic>
#include <ctime>
#include <nlohmann/json.hpp>
#include <string>
#include <tbb/concurrent_hash_map.h>
#include <thread>
#include <vector>

struct User
{
    std::string username;
    std::string password;
    std::string email;
    std::string bio;
    std::string avatar_url;
    std::vector<std::string> follower_names;
    std::vector<std::string> following_names;
    int followers;
    int followings;
    std::time_t joined_at;
    bool is_active;
    std::string active_code;

    nlohmann::json to_json() const
    {
        nlohmann::json follower_name(follower_names);
        nlohmann::json following_name(following_names);
        return {{"username", username},
                {"password", password},
                {"email", email},
                {"bio", bio},
                {"avatar_url", avatar_url},
                {"follower_names", follower_name},
                {"following_names", following_name},
                {"followers", followers},
                {"followings", followings},
                {"joined_at", joined_at},
                {"is_active", is_active}};
    }

    static User from_json(const nlohmann::json &j)
    {
        std::vector<std::string> follower_names;
        for (auto &item : j["follower_names"])
        {
            follower_names.push_back(item.get<std::string>());
        }
        std::vector<std::string> following_names;
        for (auto &item : j["following_names"])
        {
            following_names.push_back(item.get<std::string>());
        }
        return {j["username"].get<std::string>(),
                j["password"].get<std::string>(),
                j["email"].get<std::string>(),
                j["bio"].get<std::string>(),
                j["avatar_url"].get<std::string>(),
                follower_names,
                following_names,
                j["followers"].get<int>(),
                j["followings"].get<int>(),
                j["joined_at"].get<std::time_t>(),
                j["is_active"].get<bool>()};
    }
};

class UserStore
{
  public:
    using MapType = tbb::concurrent_hash_map<std::string, User>;

    UserStore(const std::string &file_path);
    ~UserStore();

    bool is_active_user(const std::string &username, const std::string code = "");
    bool active_user(const std::string &username, const std::string code);
    bool register_user(const User &user);
    bool login_user(const std::string &username, const std::string &password);
    bool get_user(const std::string &username, User &user);
    void update_avatar(const std::string &username, const std::string &avatar_url);
    void follower_user(const std::string &username, const std::string &user_id);
    void following_user(const std::string &username, const std::string &user_id);
    void unfollower_user(const std::string &username, const std::string &user_id);
    void unfollowing_user(const std::string &username, const std::string &user_id);

    void save_to_file();

  private:
    MapType users;
    std::string file_path;
    std::atomic<bool> stop_saving;
    std::thread save_thread;

    void load_from_file();
};

extern UserStore user_store;

#endif // MODELS_USER_STORE_HPP
