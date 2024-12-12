#include "models/message_store.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

MessageStore::MessageStore(const std::string &file_path) : file_path(file_path), stop_saving(false)
{
    load_from_file();
    save_thread = std::thread(&MessageStore::periodic_save, this);
}

MessageStore::~MessageStore()
{
    stop_saving = true;
    if (save_thread.joinable())
    {
        save_thread.join();
    }
    save_to_file();
}

void MessageStore::add_message(const Message &message)
{
    InboxType::accessor acc;
    if (!inbox.find(acc, message.recipient_id))
    {
        inbox.insert(acc, message.recipient_id);
    }
    acc->second.push_back(message);
}

std::vector<Message> MessageStore::get_inbox(std::string user_id)
{
    InboxType::const_accessor acc;
    if (inbox.find(acc, user_id))
    {
        return acc->second;
    }
    return {};
}

void MessageStore::save_to_file()
{
    json json_data;
    for (const auto &entry : inbox)
    {
        json messages_json = json::array();
        for (const auto &message : entry.second)
        {
            messages_json.push_back(message.to_json());
        }
        json_data[entry.first] = messages_json;
    }

    std::ofstream file(file_path);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file for saving messages");
    }
    file << json_data.dump(4);
    file.close();
}

void MessageStore::load_from_file()
{
    std::ifstream file(file_path);
    if (!file.is_open())
    {
        return; // No file to load, assume fresh start
    }

    json json_data;
    file >> json_data;
    file.close();

    for (auto it = json_data.begin(); it != json_data.end(); ++it)
    {
        std::string user_id = it.key();
        std::vector<Message> messages;
        for (const auto &msg_json : it.value())
        {
            messages.push_back(Message::from_json(msg_json));
        }
        inbox.insert({user_id, messages});
    }
}

void MessageStore::periodic_save()
{
    while (!stop_saving)
    {
        std::this_thread::sleep_for(std::chrono::seconds(60));
        save_to_file();
    }
}
