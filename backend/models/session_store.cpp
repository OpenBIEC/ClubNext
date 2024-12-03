#include "models/session_store.hpp"
#include <cstdio>
#include <random>

std::string SessionStore::generate_token() const
{
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string token;
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, sizeof(alphanum) - 2);
    for (int i = 0; i < 32; ++i)
    {
        token += alphanum[dist(gen)];
    }
    return token;
}

std::string SessionStore::create_session(const std::string &username)
{
    std::string token = generate_token();
    Session session{username, std::chrono::steady_clock::now() + std::chrono::hours(1)};
    sessions.insert({token, session});
    return token;
}

bool SessionStore::remove_session(std::string &username)
{
    for (auto it = sessions.begin(); it != sessions.end(); ++it)
    {
        MapType::accessor accessor;
        if (sessions.find(accessor, it->first) && accessor->second.username == username)
        {
            sessions.erase(accessor);
            return true;
        }
    }
    return false;
}

bool SessionStore::exist_session(std::string &username)
{
    for (auto it = sessions.begin(); it != sessions.end(); ++it)
    {
        MapType::accessor accessor;
        if (sessions.find(accessor, it->first) && accessor->second.username == username)
        {
            return true;
        }
    }
    return false;
}

bool SessionStore::validate_session(const std::string &token, std::string &username)
{
    MapType::const_accessor acc;
    if (sessions.find(acc, token))
    {
        if (std::chrono::steady_clock::now() < acc->second.expiration)
        {
            username = acc->second.username;
            return true;
        }
    }
    return false;
}

void SessionStore::cleanup_sessions()
{
    std::lock_guard<std::mutex> lock(cleanup_mutex);
    auto now = std::chrono::steady_clock::now();

    for (auto it = sessions.begin(); it != sessions.end();)
    {
        MapType::accessor acc;
        if (sessions.find(acc, it->first) && acc->second.expiration <= now)
        {

            sessions.erase(acc);
            it = sessions.begin();
        }
        else
        {
            ++it;
        }
    }
}
