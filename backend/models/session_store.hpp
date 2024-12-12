#ifndef MODELS_SESSION_STORE_HPP
#define MODELS_SESSION_STORE_HPP

#include <chrono>
#include <cstdint>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <tbb/concurrent_hash_map.h>

using json = nlohmann::json;

struct Session
{
    std::string username;
    std::chrono::time_point<std::chrono::steady_clock> expiration;

    json to_json() const
    {
        return {{"username", username},
                {"expiration",
                 std::chrono::duration_cast<std::chrono::milliseconds>(expiration.time_since_epoch()).count()}};
    }

    static Session from_json(const json &j)
    {
        Session session;
        session.username = j["username"].get<std::string>();
        session.expiration = std::chrono::time_point<std::chrono::steady_clock>(
            std::chrono::milliseconds(j["expiration"].get<int64_t>()));
        return session;
    }
};

class SessionStore
{
  public:
    using MapType = tbb::concurrent_hash_map<std::string, Session>;

    SessionStore(const std::string &file_path);
    ~SessionStore();

    bool exist_session(std::string &username);
    bool validate_session(const std::string &token, std::string &username);

    std::string create_session(const std::string &username, int remember_hour = 1);
    bool remove_session(std::string &username);
    void cleanup_sessions();

    void save_to_file();

  private:
    MapType sessions;
    std::mutex cleanup_mutex;

    std::string generate_token() const;

    std::string file_path;
    std::atomic<bool> stop_saving;
    std::thread save_thread;
    void load_from_file();
};

extern SessionStore session_store;

#endif // MODELS_SESSION_STORE_HPP
