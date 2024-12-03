#include "routes/social.hpp"
#include "models/authenticate.hpp"
#include "models/post_store.hpp"
#include "models/user_store.hpp"

void handle_follow_user(const httplib::Request &req, httplib::Response &res)
{

    auto user_id = req.matches[1];
    std::string username;

    if (!authenticate_user(req, username))
    {
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return;
    }

    User user;

    if (!user_store.get_user(user_id, user))
    {
        res.status = 404;
        res.set_content("{\"message\":\"User not found\"}", "application/json");
        return;
    }

    user_store.following_user(username, user_id);
    user_store.follower_user(user_id, username);
    user_store.save_to_file();

    res.set_content("{\"message\":\"Followed successfully\"}", "application/json");
}

void handle_unfollow_user(const httplib::Request &req, httplib::Response &res)
{
    auto user_id = req.matches[1];
    std::string username;

    if (!authenticate_user(req, username))
    {
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return;
    }

    User user;

    if (!user_store.get_user(user_id, user))
    {
        res.status = 404;
        res.set_content("{\"message\":\"User not found\"}", "application/json");
        return;
    }

    user_store.unfollowing_user(username, user_id);
    user_store.unfollower_user(user_id, username);
    user_store.save_to_file();

    res.set_content("{\"message\":\"Unfollowed successfully\"}", "application/json");
}

void handle_like_post(const httplib::Request &req, httplib::Response &res)
{
    auto post_id = std::stoi(req.matches[1]);
    Post post;

    if (!post_store.get_post(post_id, post))
    {
        res.status = 404;
        res.set_content("{\"message\":\"Post not found\"}", "application/json");
        return;
    }

    post.like_count++;
    post_store.save_to_file();

    res.set_content("{\"message\":\"Post liked successfully\"}", "application/json");
}

void handle_unlike_post(const httplib::Request &req, httplib::Response &res)
{
    auto post_id = std::stoi(req.matches[1]);
    Post post;

    if (!post_store.get_post(post_id, post))
    {
        res.status = 404;
        res.set_content("{\"message\":\"Post not found\"}", "application/json");
        return;
    }

    if (post.like_count > 0)
        post.like_count--;
    post_store.save_to_file();

    res.set_content("{\"message\":\"Post unliked successfully\"}", "application/json");
}
