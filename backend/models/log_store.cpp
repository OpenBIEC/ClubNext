#include "log_store.hpp"
#include "config.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

LogStore::LogStore(const std::string &file_path) : file_path(file_path), stop_saving(false)
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

LogStore::~LogStore()
{
    stop_saving = true;
    if (save_thread.joinable())
    {
        save_thread.join();
    }
    save_to_file();
}

void LogStore::add_log(LogLevel level, const std::string &message)
{
    LogEntry entry = {std::time(nullptr), level, message};
    logs.push_back(entry);
}

std::vector<LogEntry> LogStore::get_logs(LogLevel level)
{
    std::vector<LogEntry> filtered_logs;
    for (const auto &log : logs)
    {
        if (log.level == level)
        {
            filtered_logs.push_back(log);
        }
    }
    return filtered_logs;
}

void LogStore::save_to_file()
{
    std::lock_guard<std::mutex> lock(save_mutex);
    std::ofstream file(file_path, std::ios::trunc);

    if (!file.is_open())
    {
        std::cerr << "Failed to open log file for saving: " << file_path << std::endl;
        return;
    }

    json j_logs = json::array();
    for (const auto &log : logs)
    {
        j_logs.push_back(log.to_json());
    }

    file << j_logs.dump(4);
    file.close();
}

void LogStore::load_from_file()
{
    if (is_saving.test_and_set())
    {
        return;
    }

    try
    {
        json json_data;
        std::ifstream file(file_path);
        if (file.is_open())
        {
            file >> json_data;
            for (const auto &item : json_data)
            {
                LogEntry log_entry = LogEntry::from_json(item);
                logs.push_back(log_entry);
            }
            file.close();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading log store from file: " << e.what() << std::endl;
    }
}
