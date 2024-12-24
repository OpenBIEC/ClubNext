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
    INFO_LOG,
    WARNING_LOG,
    ERROR_LOG,
    DEBUG_LOG
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
    using VectorType = tbb::concurrent_vector<LogEntry>;

    LogStore(const std::string &file_path);
    ~LogStore();

    void add_log(LogLevel level, const std::string &message);
    std::vector<LogEntry> get_logs(LogLevel level);
    void save_to_file();

  private:
    VectorType logs;
    std::string file_path;
    std::atomic<bool> stop_saving;
    std::thread save_thread;
    std::atomic_flag is_saving = ATOMIC_FLAG_INIT;
    std::mutex save_mutex;

    void load_from_file();
};

extern LogStore log_store;

#endif // LOG_STORE_HPP
