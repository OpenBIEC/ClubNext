#include "models/draft_store.hpp"
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace fs = std::filesystem;

DraftStore::DraftStore(const std::string &file_path) : file_path(file_path)
{
    load_from_file();
}

DraftStore::~DraftStore()
{
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
    std::ofstream file(file_path);
    if (!file.is_open())
    {
        throw std::runtime_error("Unable to open draft store file for saving.");
    }

    json j;
    for (const auto &[username, user_drafts] : drafts)
    {
        json draft_map = json::object();
        for (const auto &[draft_id, draft] : user_drafts)
        {
            draft_map[std::to_string(draft_id)] = draft.to_json();
        }
        j[username] = draft_map;
    }

    file << j.dump(4);
    file.close();
}

void DraftStore::load_from_file()
{
    if (!fs::exists(file_path))
    {
        return;
    }

    std::ifstream file(file_path);
    if (!file.is_open())
    {
        throw std::runtime_error("Unable to open draft store file for loading.");
    }

    json j;
    file >> j;
    file.close();

    for (const auto &[username, draft_map] : j.items())
    {
        DraftMapType user_drafts;
        for (const auto &[draft_id_str, draft_json] : draft_map.items())
        {
            Draft draft = Draft::from_json(draft_json);
            user_drafts.insert({std::stoi(draft_id_str), draft});
        }
        drafts.insert({username, user_drafts});
    }
}

DraftStore::UserMapType &DraftStore::get_all_drafts()
{
    return drafts;
}
