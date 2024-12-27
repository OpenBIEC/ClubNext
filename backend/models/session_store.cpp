#include "models/session_store.hpp"
#include "config.hpp"
#include "session_store.hpp"
#include <fstream>
#include <iostream>
#include <random>

SessionStore::SessionStore(const std::string &file_path) : file_path(file_path), stop_saving(false)
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

SessionStore::~SessionStore()
{
    stop_saving.store(true);
    if (save_thread.joinable())
    {
        save_thread.join();
    }
    save_to_file();
}

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

std::string SessionStore::create_session(const std::string &username, int remember_hour)
{
    std::string token = generate_token();
    Session session{username, std::chrono::steady_clock::now() + std::chrono::hours(remember_hour)};
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

void SessionStore::save_to_file()
{
    if (is_saving.test_and_set())
    {
        return;
    }

    try
    {
        json json_data = json::array();
        for (const auto &item : sessions)
        {
            json_data.push_back({{"token", item.first}, {"session", item.second.to_json()}});
        }
        std::ofstream file(file_path, std::ios::out | std::ios::trunc);
        file << json_data.dump(4);
        file.close();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error saving session store to file: " << e.what() << std::endl;
    }
}

void SessionStore::load_from_file()
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
                auto session = Session::from_json(item["session"]);
                MapType::accessor acc;
                sessions.insert(acc, item["token"].get<std::string>());
                acc->second = session;
            }
            file.close();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading session store from file: " << e.what() << std::endl;
    }
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
