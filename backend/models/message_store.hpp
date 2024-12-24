#ifndef MODELS_MESSAGE_STORE_HPP
#define MODELS_MESSAGE_STORE_HPP

#include <atomic>
#include <ctime>
#include <nlohmann/json.hpp>
#include <string>
#include <tbb/concurrent_hash_map.h>
#include <thread>
#include <vector>

using json = nlohmann::json;

struct Message
{
    std::string sender_id;
    std::string recipient_id;
    std::string content;
    std::time_t timestamp;

    json to_json() const
    {
        return {
            {"sender_id", sender_id}, {"recipient_id", recipient_id}, {"content", content}, {"timestamp", timestamp}};
    }

    static Message from_json(const json &j)
    {
        return {j["sender_id"].get<std::string>(), j["recipient_id"].get<std::string>(),
                j["content"].get<std::string>(), j["timestamp"].get<std::time_t>()};
    }
};

class MessageStore
{
  public:
    using InboxType = tbb::concurrent_hash_map<std::string, std::vector<Message>>;

    MessageStore(const std::string &file_path);
    ~MessageStore();

    void add_message(const Message &message);

    std::vector<Message> get_inbox(std::string user_id);

    void save_to_file();

  private:
    InboxType inbox;
    std::string file_path;
    std::atomic<bool> stop_saving;
    std::thread save_thread;
    std::atomic_flag is_saving = ATOMIC_FLAG_INIT;

    void load_from_file();
};

extern MessageStore message_store;

#endif // MODELS_MESSAGE_STORE_HPP
