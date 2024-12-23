#ifndef LOG_STORE_HPP
#define LOG_STORE_HPP

#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>
#include <vector>

using json = nlohmann::json;

enum class LogLevel
{
    INFO,
    WARNING,
    ERROR,
    DEBUG
};

struct LogEntry
{
    std::time_t timestamp;
    LogLevel level;
    std::string message;

    json to_json() const
    {
        return {{"timestamp", timestamp}, {"level", static_cast<int>(level)}, {"message", message}};
    }

    static LogEntry from_json(const json &j)
    {
        return {j["timestamp"].get<std::time_t>(), static_cast<LogLevel>(j["level"].get<int>()),
                j["message"].get<std::string>()};
    }
};

class LogStore
{
  public:
    LogStore(const std::string &file_path);
    ~LogStore();

    void add_log(LogLevel level, const std::string &message);
    std::vector<LogEntry> get_logs(LogLevel level);
    void save_to_file();
    void load_from_file();

  private:
    tbb::concurrent_vector<LogEntry> logs;
    std::string file_path;
    std::atomic<bool> stop_saving;
    std::thread save_thread;
    std::mutex save_mutex;

    void periodic_save();
};

extern LogStore log_store;

#endif // LOG_STORE_HPP
