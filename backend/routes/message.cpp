#include "routes/message.hpp"
#include "config.hpp"
#include "models/authenticate.hpp"
#include "models/message_store.hpp"
#include "models/user_store.hpp"

void handle_get_messages(const httplib::Request &req, httplib::Response &res)
{
    std::string user_id;
    if (!authenticate_user(req, user_id))
    {
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return;
    }
    auto messages = message_store.get_inbox(user_id);

    nlohmann::json response = {{"messages", nlohmann::json::array()}};
    for (const auto &msg : messages)
    {
        response["messages"].push_back(msg.to_json());
    }

    res.set_content(response.dump(), "application/json");
}

void handle_send_message(const httplib::Request &req, httplib::Response &res)
{
    nlohmann::json body;
    try
    {
        body = nlohmann::json::parse(req.body);
    }
    catch (const std::exception &)
    {
        res.status = 400;
        res.set_content("{\"message\":\"Invalid JSON\"}", "application/json");
        return;
    }

    std::string sender_id;
    if (!authenticate_user(req, sender_id))
    {
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return;
    }

    std::string recipient_id = body["recipient_id"].get<std::string>();
    std::string content = body["content"].get<std::string>();

    User user;
    if (!user_store.get_user(recipient_id, user))
    {
        res.status = 404;
        res.set_content("{\"error\":\"User not found\"}", "application/json");
        return;
    }

    if (std::ranges::find(user.following_names, sender_id) == user.following_names.end())
    {
        res.status = 403;
        res.set_content("{\"error\":\"Forbidden\"}", "application/json");
        return;
    }

    if (content.length() > config.MAX_MESSAGE_LENGTH)
    {
        content = content.substr(0, config.MAX_MESSAGE_LENGTH);
    }

    Message new_message = {sender_id, recipient_id, content, std::time(nullptr)};
    message_store.add_message(new_message);

    res.set_content("{\"message\":\"Message sent successfully\"}", "application/json");
}
