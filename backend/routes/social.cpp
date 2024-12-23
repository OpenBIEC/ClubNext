#include "routes/social.hpp"
#include "models/authenticate.hpp"
#include "models/log_store.hpp"
#include "models/post_store.hpp"
#include "models/user_store.hpp"
#include <algorithm>
#include <string>


void handle_follow_user(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing follow user request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized follow user request");
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return;
    }

    auto user_id = req.matches[1].str();
    User user;
    if (!user_store.get_user(user_id, user))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Follow user failed: User not found - " + user_id);
        res.status = 404;
        res.set_content("{\"message\":\"User not found\"}", "application/json");
        return;
    }

    if (std::ranges::find(user.following_names, username) != user.following_names.end())
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Follow user failed: Already following user - " + user_id);
        res.status = 403;
        res.set_content("{\"error\":\"Forbidden\"}", "application/json");
        return;
    }

    user_store.following_user(username, user_id);
    user_store.follower_user(user_id, username);
    user_store.save_to_file();

    log_store.add_log(LogLevel::INFO_LOG, "User followed successfully: " + user_id);
    res.set_content("{\"message\":\"Followed successfully\"}", "application/json");
}

void handle_unfollow_user(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing unfollow user request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized unfollow user request");
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return;
    }

    auto user_id = req.matches[1].str();
    User user;
    if (!user_store.get_user(user_id, user))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unfollow user failed: User not found - " + user_id);
        res.status = 404;
        res.set_content("{\"message\":\"User not found\"}", "application/json");
        return;
    }

    if (std::ranges::find(user.following_names, username) == user.following_names.end())
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unfollow user failed: Not following user - " + user_id);
        res.status = 403;
        res.set_content("{\"error\":\"Forbidden\"}", "application/json");
        return;
    }

    user_store.unfollowing_user(username, user_id);
    user_store.unfollower_user(user_id, username);
    user_store.save_to_file();

    log_store.add_log(LogLevel::INFO_LOG, "User unfollowed successfully: " + user_id);
    res.set_content("{\"message\":\"Unfollowed successfully\"}", "application/json");
}

void handle_like_post(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing like post request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized like post request");
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return;
    }

    auto post_id = std::stoi(req.matches[1].str());
    PostStore::MapType::accessor acc;
    if (!post_store.get_acc(post_id, acc))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Like post failed: Post not found - " + std::to_string(post_id));
        res.status = 404;
        res.set_content("{\"message\":\"Post not found\"}", "application/json");
        return;
    }
    Post &post = acc->second;

    if (std::ranges::find(post.liked_by_users, username) != post.liked_by_users.end())
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Like post failed: Already liked by user - " + username);
        res.status = 403;
        res.set_content("{\"error\":\"Forbidden\"}", "application/json");
        return;
    }

    post.like_count++;
    post.liked_by_users.push_back(username);
    post_store.save_to_file();

    log_store.add_log(LogLevel::INFO_LOG, "Post liked successfully: " + std::to_string(post_id));
    res.set_content("{\"message\":\"Post liked successfully\"}", "application/json");
}

void handle_unlike_post(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing unlike post request");
    std::string username;
    if (!authenticate_user(req, username))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized unlike post request");
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return;
    }

    auto post_id = std::stoi(req.matches[1].str());
    PostStore::MapType::accessor acc;
    if (!post_store.get_acc(post_id, acc))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unlike post failed: Post not found - " + std::to_string(post_id));
        res.status = 404;
        res.set_content("{\"message\":\"Post not found\"}", "application/json");
        return;
    }
    Post &post = acc->second;

    if (std::ranges::find(post.liked_by_users, username) == post.liked_by_users.end())
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unlike post failed: Not liked by user - " + username);
        res.status = 403;
        res.set_content("{\"error\":\"Forbidden\"}", "application/json");
        return;
    }

    post.like_count--;
    std::ranges::find(post.liked_by_users, username)->clear();
    post_store.save_to_file();

    log_store.add_log(LogLevel::INFO_LOG, "Post unliked successfully: " + std::to_string(post_id));
    res.set_content("{\"message\":\"Post unliked successfully\"}", "application/json");
}
