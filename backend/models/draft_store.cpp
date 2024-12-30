#include "models/draft_store.hpp"
#include "config.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>

DraftStore::DraftStore(const std::string &file_path) : file_path(file_path), stop_saving(false)
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

DraftStore::~DraftStore()
{
    stop_saving.store(true);
    if (save_thread.joinable())
    {
        save_thread.join();
    }
    save_to_file();
}

bool DraftStore::add_draft(const std::string &username, const Draft &draft)
{
    UserMapType::accessor user_acc;
    drafts.insert(user_acc, username);

    DraftMapType::accessor draft_acc;
    user_acc->second.insert(draft_acc, draft.id);

    draft_acc->second = draft;
    return true;
}

bool DraftStore::get_draft(const std::string &username, int id, Draft &draft)
{
    UserMapType::const_accessor user_acc;
    if (drafts.find(user_acc, username))
    {
        DraftMapType::const_accessor draft_acc;
        if (user_acc->second.find(draft_acc, id))
        {
            draft = draft_acc->second;
            return true;
        }
    }
    return false;
}

bool DraftStore::set_draft(const std::string &username, int id, const Draft &draft)
{
    UserMapType::accessor user_acc;
    if (drafts.find(user_acc, username))
    {
        DraftMapType::accessor draft_acc;
        if (user_acc->second.find(draft_acc, id))
        {
            draft_acc->second = draft;
            return true;
        }
    }
    return false;
}

bool DraftStore::delete_draft(const std::string &username, int id)
{
    UserMapType::accessor user_acc;
    if (drafts.find(user_acc, username))
    {
        DraftMapType::accessor draft_acc;
        if (user_acc->second.find(draft_acc, id))
        {
            user_acc->second.erase(draft_acc);
            return true;
        }
    }
    return false;
}

void DraftStore::save_to_file()
{
    if (is_saving.test_and_set())
    {
        return;
    }

    try
    {
        json json_data = json::array();
        for (const auto &[username, user_drafts] : drafts)
        {
            for (const auto &item : user_drafts)
            {
                json_data.push_back(item.second.to_json());
            }
        }
        json j = {{"drafts", json_data}, {"count", draft_count.load()}};
        std::ofstream file(file_path, std::ios::out | std::ios::trunc);
        file << j.dump(4);
        file.close();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error saving draft store to file: " << e.what() << std::endl;
    }
}

void DraftStore::load_from_file()
{
    try
    {
        std::ifstream file(file_path);
        if (file.is_open())
        {
            json j;
            file >> j;
            draft_count = j["count"].get<size_t>();
            json json_data = j["drafts"];
            for (const auto &item : json_data)
            {
                Draft draft = Draft::from_json(item);
                add_draft(draft.author, draft);
            }
            file.close();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading post store from file: " << e.what() << std::endl;
    }
}

DraftStore::UserMapType &DraftStore::get_drafts()
{
    return drafts;
}
