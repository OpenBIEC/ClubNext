#include "models/comment_store.hpp"
#include "comment_store.hpp"
#include "config.hpp"
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace fs = std::filesystem;

CommentStore::CommentStore()
{
    if (!fs::exists(Config::COMMENT_DIR))
    {
        fs::create_directories(Config::COMMENT_DIR);
    }
    load_from_file();
}

CommentStore::~CommentStore()
{
    save_to_file();
}

bool CommentStore::add_comment(int post_id, const Comment &comment)
{
    MapType::accessor acc;
    if (!comments_map.find(acc, post_id))
    {
        comments_map.insert(acc, post_id);
    }
    acc->second.push_back(comment);
    save_to_file();
    return true;
}

bool CommentStore::edit_comment(int post_id, int comment_id, const std::string &new_content)
{
    MapType::accessor acc;
    if (comments_map.find(acc, post_id))
    {
        for (auto &comment : acc->second)
        {
            if (comment.comment_id == comment_id && !comment.is_deleted)
            {
                std::ofstream file(get_comment_directory(post_id, comment_id) + "/content.md");
                if (!file.is_open())
                {
                    throw std::runtime_error("Failed to open content file for editing");
                }
                file << new_content;
                file.close();
                return true;
            }
        }
    }
    return false;
}

bool CommentStore::delete_comment(int post_id, int comment_id)
{
    MapType::accessor acc;
    if (comments_map.find(acc, post_id))
    {
        for (auto &comment : acc->second)
        {
            if (comment.comment_id == comment_id && !comment.is_deleted)
            {
                comment.is_deleted = true;
                save_to_file();
                return true;
            }
        }
    }
    return false;
}

bool CommentStore::get_comment(int post_id, int comment_id, Comment &comment)
{
    MapType::accessor acc;
    if (comments_map.find(acc, post_id))
    {
        for (auto &item : acc->second)
        {
            if (item.comment_id == comment_id)
            {
                comment = item;
                return true;
            }
        }
    }
    return false;
}

std::vector<Comment> CommentStore::get_comments(int post_id)
{
    MapType::const_accessor acc;
    if (comments_map.find(acc, post_id))
    {
        std::vector<Comment> visible_comments;
        for (const auto &comment : acc->second)
        {
            if (!comment.is_deleted)
            {
                visible_comments.push_back(comment);
            }
        }
        return visible_comments;
    }
    return {};
}

void CommentStore::save_to_file()
{
    for (const auto &entry : comments_map)
    {
        nlohmann::json json_data = nlohmann::json::array();
        for (const auto &comment : entry.second)
        {
            json_data.push_back(comment.to_json());
        }

        std::ofstream file(get_post_comment_file(entry.first));
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file for saving comments");
        }
        file << json_data.dump(4);
        file.close();
    }
}

void CommentStore::load_from_file()
{
    for (const auto &file : fs::directory_iterator(Config::COMMENT_DIR))
    {
        if (file.path().extension() == ".json")
        {
            int post_id = std::stoi(file.path().stem().string().substr(5)); // 提取 post_id
            std::ifstream in_file(file.path());
            if (!in_file.is_open())
                continue;

            nlohmann::json json_data;
            in_file >> json_data;
            in_file.close();

            VectorType comments;
            for (const auto &item : json_data)
            {
                comments.push_back(Comment::from_json(item));
            }
            comments_map.insert({post_id, comments});
        }
    }
}

std::string CommentStore::get_post_comment_file(int post_id)
{
    return Config::COMMENT_DIR + "/post_" + std::to_string(post_id) + ".json";
}

std::string CommentStore::get_comment_directory(int post_id, int comment_id)
{
    return Config::COMMENT_DIR + "/post_" + std::to_string(post_id) + "/comment_" + std::to_string(comment_id);
}
