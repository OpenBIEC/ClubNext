#include "routes/message.hpp"
#include "config.hpp"
#include "models/authenticate.hpp"
#include "models/log_store.hpp"
#include "models/message_store.hpp"
#include "models/user_store.hpp"


void handle_get_messages(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing get messages request");
    std::string user_id;
    if (!authenticate_user(req, user_id))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized get messages request");
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return;
    }
    auto messages = message_store.get_inbox(user_id);

    json response = {"messages", json::array()};
    for (const auto &msg : messages)
    {
        response["messages"].push_back(msg.to_json());
    }

    log_store.add_log(LogLevel::INFO_LOG, "Messages retrieved successfully for user: " + user_id);
    res.set_content(response.dump(), "application/json");
}

void handle_send_message(const httplib::Request &req, httplib::Response &res)
{
    log_store.add_log(LogLevel::INFO_LOG, "Processing send message request");
    json body;
    try
    {
        body = json::parse(req.body);
    }
    catch (const std::exception &)
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Invalid JSON in send message request");
        res.status = 400;
        res.set_content("{\"message\":\"Invalid JSON\"}", "application/json");
        return;
    }

    std::string sender_id;
    if (!authenticate_user(req, sender_id))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Unauthorized send message request");
        res.status = 401;
        res.set_content(R"({"error":"Unauthorized"})", "application/json");
        return;
    }

    std::string recipient_id = body["recipient_id"].get<std::string>();
    std::string content = body["content"].get<std::string>();

    User user;
    if (!user_store.get_user(recipient_id, user))
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Recipient user not found: " + recipient_id);
        res.status = 404;
        res.set_content("{\"error\":\"User not found\"}", "application/json");
        return;
    }

    if (std::ranges::find(user.following_names, sender_id) == user.following_names.end())
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Sender is not allowed to send message to recipient: " + recipient_id);
        res.status = 403;
        res.set_content("{\"error\":\"Forbidden\"}", "application/json");
        return;
    }

    if (content.length() > config.MAX_MESSAGE_LENGTH)
    {
        log_store.add_log(LogLevel::WARNING_LOG, "Message content truncated to max length for sender: " + sender_id);
        content = content.substr(0, config.MAX_MESSAGE_LENGTH);
    }

    Message new_message = {sender_id, recipient_id, content, std::time(nullptr)};
    message_store.add_message(new_message);

    log_store.add_log(LogLevel::INFO_LOG,
                      "Message sent successfully from sender: " + sender_id + " to recipient: " + recipient_id);
    res.set_content("{\"message\":\"Message sent successfully\"}", "application/json");
}
