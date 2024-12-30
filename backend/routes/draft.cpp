#include "routes/draft.hpp"
#include "models/authenticate.hpp"
#include "models/draft_store.hpp"
#include "models/log_store.hpp"
#include "models/post_store.hpp"
#include "models/user_store.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void handle_get_drafts(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing get drafts request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized get drafts request");
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    DraftStore::UserMapType::const_accessor user_acc;
    if (!draft_store.get_drafts().find(user_acc, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "No drafts found for user: " + username);
        res.status = 404;
        res.set_content("{\"error\":\"No drafts found\"}", "application/json");
        return;
    }

    json response = json::array();
    for (const auto &[draft_id, draft] : user_acc->second)
    {
        response.push_back(draft.to_json());
    }

    log_store.add_log(LogLevel::INFO_LOG, "Drafts retrieved successfully for user: " + username);
    res.set_content(response.dump(), "application/json");
}

void handle_create_draft(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing create draft request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized create draft request");
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    json body = json::parse(req.body, nullptr, false);
    if (body.is_discarded() || !body.contains("content"))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Invalid JSON in create draft request");
        res.status = 400;
        res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
        return;
    }

    std::string content = body["content"].get<std::string>();
    int draft_id = draft_store.draft_count.fetch_add(1);

    std::string draft_dir = "drafts/" + username + "/" + std::to_string(draft_id);
    std::string content_file_path = draft_dir + "/" + "content.md";

    if (!std::filesystem::create_directories(draft_dir))
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Failed to create draft directory: " + draft_dir);
        res.status = 500;
        res.set_content("{\"error\":\"Failed to create draft directory\"}", "application/json");
        return;
    }
    std::ofstream content_file(content_file_path);
    if (!content_file)
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Failed to create content file: " + content_file_path);
        res.status = 500;
        res.set_content("{\"error\":\"Failed to create content file\"}", "application/json");
        return;
    }
    content_file << content;
    content_file.close();

    Draft new_draft(draft_id, username, content_file_path);
    draft_store.add_draft(username, new_draft);

    log_store.add_log(LogLevel::INFO_LOG, "Draft created successfully with ID: " + std::to_string(draft_id));
    res.set_content(json{{"message", "Draft created successfully"}, {"draft_id", draft_id}}.dump(), "application/json");
}

void handle_edit_draft(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing edit draft request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized edit draft request");
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    json body = json::parse(req.body, nullptr, false);
    if (body.is_discarded() || !body.contains("draft_id") || !body.contains("content"))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Invalid JSON in edit draft request");
        res.status = 400;
        res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
        return;
    }

    int draft_id = body["draft_id"].get<int>();
    std::string new_content = body["content"].get<std::string>();

    Draft draft;
    if (draft_store.get_draft(username, draft_id, draft))
    {
        draft.content = new_content;
        draft.timestamp = std::time(nullptr);
        draft_store.set_draft(username, draft_id, draft);

        log_store.add_log(LogLevel::INFO_LOG, "Draft edited successfully for user: " + username);
        res.set_content("{\"message\":\"Draft updated successfully\"}", "application/json");
    }
    else
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Draft not found for user: " + username);
        res.status = 404;
        res.set_content("{\"error\":\"Draft not found\"}", "application/json");
    }
}

void handle_delete_draft(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing delete draft request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized delete draft request");
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    json body = json::parse(req.body, nullptr, false);
    if (body.is_discarded() || !body.contains("draft_id"))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Invalid JSON in delete draft request");
        res.status = 400;
        res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
        return;
    }

    int draft_id = body["draft_id"].get<int>();

    if (draft_store.delete_draft(username, draft_id))
    {
        log_store.add_log(LogLevel::INFO_LOG, "Draft deleted successfully for user: " + username);
        res.set_content("{\"message\":\"Draft deleted successfully\"}", "application/json");
    }
    else
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Draft not found for user: " + username);
        res.status = 404;
        res.set_content("{\"error\":\"Draft not found\"}", "application/json");
    }
}

void handle_upload_draft_media(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing upload draft media request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized upload draft media request");
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    json body = json::parse(req.body, nullptr, false);
    if (body.is_discarded() || !body.contains("draft_id"))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Invalid JSON in upload draft media request");
        res.status = 400;
        res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
        return;
    }

    int draft_id = body["draft_id"].get<int>();

    Draft draft;
    if (!draft_store.get_draft(username, draft_id, draft))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Draft not found for user: " + username);
        res.status = 404;
        res.set_content("{\"error\":\"Draft not found\"}", "application/json");
        return;
    }

    auto file = req.get_file_value("media");
    if (file.filename.empty())
    {
        log_store.add_log(LogLevel::WARNING_LOG, "No file uploaded in draft media request");
        res.status = 400;
        res.set_content("{\"error\":\"No file uploaded\"}", "application/json");
        return;
    }

    if (!user_store.use_space(username, file.content.size()))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "User file space exceeded for user: " + username);
        res.status = 400;
        res.set_content("{\"error\":\"User file space exceeds limit of 1GB\"}", "application/json");
        return;
    }

    std::string draft_dir = "drafts/" + username + "/" + std::to_string(draft_id) + "/";
    std::string media_path = draft_dir + file.filename;

    std::ofstream media_file(media_path, std::ios::binary);
    if (!media_file)
    {
        log_store.add_log(LogLevel::ERROR_LOG, "Failed to save media file: " + media_path);
        res.status = 500;
        res.set_content("{\"error\":\"Failed to save media file\"}", "application/json");
        return;
    }
    media_file.write(file.content.data(), file.content.size());
    media_file.close();

    draft.media.push_back(media_path);
    draft_store.set_draft(username, draft_id, draft);

    log_store.add_log(LogLevel::INFO_LOG, "Media uploaded successfully for draft ID: " + std::to_string(draft_id));
    res.set_content(json{{"message", "Media uploaded successfully"}, {"path", media_path}}.dump(), "application/json");
}

void handle_publish_draft(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing publish draft request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized publish draft request");
        res.status = 401;
        res.set_content("{\"error\":\"Unauthorized\"}", "application/json");
        return;
    }

    json body = json::parse(req.body, nullptr, false);
    if (body.is_discarded() || !body.contains("draft_id"))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Invalid JSON in publish draft request");
        res.status = 400;
        res.set_content("{\"error\":\"Invalid JSON\"}", "application/json");
        return;
    }

    int draft_id = body["draft_id"].get<int>();

    Draft draft;
    if (!draft_store.get_draft(username, draft_id, draft))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Draft not found for user: " + username);
        res.status = 404;
        res.set_content("{\"error\":\"Draft not found\"}", "application/json");
        return;
    }

    Post new_post;
    new_post.id = ++post_store.post_count;
    new_post.author = draft.author;
    new_post.content = draft.content;
    new_post.media = draft.media;
    new_post.tags = {};
    new_post.like_count.store(0);
    new_post.comment_count.store(0);
    new_post.created_at = std::time(nullptr);

    post_store.add_post(new_post);
    draft_store.delete_draft(username, draft_id);

    log_store.add_log(LogLevel::INFO_LOG, "Draft published successfully for user: " + username);
    res.set_content(json{{"message", "Draft published successfully"}, {"post_id", new_post.id}}.dump(),
                    "application/json");
}
