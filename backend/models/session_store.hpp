#ifndef MODELS_SESSION_STORE_HPP
#define MODELS_SESSION_STORE_HPP

#include <chrono>
#include <mutex>
#include <string>
#include <tbb/concurrent_hash_map.h>

struct Session
{
    std::string username;
    std::chrono::time_point<std::chrono::steady_clock> expiration;
};

class SessionStore
{
  public:
    using MapType = tbb::concurrent_hash_map<std::string, Session>;

    std::string create_session(const std::string &username, int remember_hour = 1);
    bool remove_session(std::string &username);

    bool exist_session(std::string &username);
    bool validate_session(const std::string &token, std::string &username);

    void cleanup_sessions();

  private:
    MapType sessions;
    std::mutex cleanup_mutex;

    std::string generate_token() const;
};

extern SessionStore session_store;

#endif // MODELS_SESSION_STORE_HPP
