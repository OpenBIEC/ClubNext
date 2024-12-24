#ifndef MODELS_USER_STORE_HPP
#define MODELS_USER_STORE_HPP

#include <atomic>
#include <cstddef>
#include <ctime>
#include <nlohmann/json.hpp>
#include <string>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>
#include <thread>

using json = nlohmann::json;

struct User
{
    using VectorType = tbb::concurrent_vector<std::string>;

    std::string username;
    std::string password;
    std::string email;
    std::string bio;
    std::string avatar_url;
    VectorType follower_names;
    VectorType following_names;
    std::atomic<int> followers;
    std::atomic<int> followings;
    std::atomic<size_t> user_space;
    std::time_t joined_at;
    int user_mode;
    std::string active_code;

    User() : followers(0), followings(0), user_space(0), joined_at(std::time(nullptr)), user_mode(0)
    {
    }

    User(const std::string &uname, const std::string &pwd, const std::string &mail, const std::string &bio_text,
         const std::string &avatar, const VectorType &followers_list, const VectorType &following_list,
         int follower_count, int following_count, size_t userspace, std::time_t join_time, int usermode,
         const std::string &activation_code = "")
        : username(uname), password(pwd), email(mail), bio(bio_text), avatar_url(avatar),
          follower_names(followers_list), following_names(following_list), followers(follower_count),
          followings(following_count), user_space(userspace), joined_at(join_time), user_mode(usermode),
          active_code(activation_code)
    {
    }

    User(const User &other)
        : username(other.username), password(other.password), email(other.email), bio(other.bio),
          avatar_url(other.avatar_url), follower_names(other.follower_names), following_names(other.following_names),
          followers(other.followers.load()), followings(other.followings.load()), user_space(other.user_space.load()),
          joined_at(other.joined_at), user_mode(other.user_mode), active_code(other.active_code)
    {
    }

    User &operator=(const User &other)
    {
        if (this == &other)
            return *this;

        username = other.username;
        password = other.password;
        email = other.email;
        bio = other.bio;
        avatar_url = other.avatar_url;
        follower_names = other.follower_names;
        following_names = other.following_names;

        followers.store(other.followers.load());
        followings.store(other.followings.load());
        user_space.store(other.user_space.load());

        joined_at = other.joined_at;
        user_mode = other.user_mode;
        active_code = other.active_code;

        return *this;
    }

    json to_json() const
    {
        std::vector<std::string> follower_name_list(follower_names.begin(), follower_names.end());
        std::vector<std::string> following_name_list(following_names.begin(), following_names.end());

        return {{"username", username},
                {"password", password},
                {"email", email},
                {"bio", bio},
                {"avatar_url", avatar_url},
                {"follower_names", follower_name_list},
                {"following_names", following_name_list},
                {"followers", followers.load()},
                {"followings", followings.load()},
                {"user_space", user_space.load()},
                {"joined_at", joined_at},
                {"user_mode", user_mode}};
    }

    static User from_json(const json &j)
    {
        User user;
        user.username = j["username"].get<std::string>();
        user.password = j["password"].get<std::string>();
        user.email = j["email"].get<std::string>();
        user.bio = j["bio"].get<std::string>();
        user.avatar_url = j["avatar_url"].get<std::string>();

        for (const auto &item : j["follower_names"])
        {
            user.follower_names.push_back(item.get<std::string>());
        }
        for (const auto &item : j["following_names"])
        {
            user.following_names.push_back(item.get<std::string>());
        }

        user.followers.store(j["followers"].get<int>());
        user.followings.store(j["followings"].get<int>());
        user.user_space.store(j["user_space"].get<size_t>());
        user.joined_at = j["joined_at"].get<std::time_t>();
        user.user_mode = j["user_mode"].get<int>();

        return user;
    }
};

class UserStore
{
  public:
    using MapType = tbb::concurrent_hash_map<std::string, User>;

    UserStore(const std::string &file_path);
    ~UserStore();

    bool accept_mode(const std::string &username, int mode = 0);
    bool store_active_code(const std::string &username, const std::string code);
    bool active_user(const std::string &username, const std::string code);
    bool register_user(const User &user);
    bool login_user(const std::string &username, const std::string &password);
    bool get_user(const std::string &username, User &user);
    bool use_space(const std::string &username, size_t file_size);
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
    std::atomic_flag is_saving = ATOMIC_FLAG_INIT;

    void load_from_file();
};

extern UserStore user_store;

#endif // MODELS_USER_STORE_HPP
