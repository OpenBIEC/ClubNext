#include "config.hpp"
#include "models/comment_store.hpp"
#include "models/draft_store.hpp"
#include "models/log_store.hpp"
#include "models/message_store.hpp"
#include "models/post_store.hpp"
#include "models/session_store.hpp"
#include "models/tag_store.hpp"
#include "models/user_store.hpp"
#include "routes/comment.hpp"
#include "routes/draft.hpp"
#include "routes/forum.hpp"
#include "routes/message.hpp"
#include "routes/social.hpp"
#include "routes/user.hpp"
#include <chrono>
#include <csignal>
#include <httplib.h>
#include <iostream>
#include <thread>

Config config;

SessionStore session_store(config.SESSION_STORE);

void session_cleanup_task()
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::minutes(10));
        session_store.cleanup_sessions();
    }
}

UserStore user_store(config.USER_STORE);
PostStore post_store(config.POST_STORE);
TagStore tag_store(config.TAG_STORE);
MessageStore message_store(config.MESSAGE_STORE);
LogStore log_store(config.LOG_STORE);
DraftStore draft_store(config.DRAFT_STORE);
CommentStore comment_store;

void signal_handler(int signal)
{
    if (signal == SIGINT)
    {
        std::cout << "\nReceived SIGINT, saving user data to file..." << std::endl;
        log_store.save_to_file();
        user_store.save_to_file();
        post_store.save_to_file();
        draft_store.save_to_file();
        tag_store.save_to_file();
        message_store.save_to_file();
        comment_store.save_to_file();
        session_store.save_to_file();
        std::cout << "Data saved successfully. Exiting..." << std::endl;
    }
}

int main()
{
    std::signal(SIGINT, signal_handler);

    std::thread cleanup_thread(session_cleanup_task);
    cleanup_thread.detach();

    httplib::Server server;

    server.set_mount_point("/", config.STATIC_DIR);
    server.set_mount_point("/posts", config.POST_DIR);

    server.Post("/api/user/register", handle_user_register);
    server.Post("/api/user/login", handle_user_login);
    server.Get("/api/user/profile", handle_user_profile);
    server.Post("/api/user/profile/update", handle_user_update_profile);
    server.Post("/api/user/avatar", handle_user_update_avatar);
    server.Get(R"(/api/user/(\w+)/profile)", handle_user_get_profile);
    server.Get("/api/user/verify", send_verify_email);
    server.Get("/api/user/validate", verify_user_email);

    server.Get(R"(/api/forum/post/(\d+))", handle_get_post_detail);
    server.Post("/api/forum/post", handle_create_post);
    server.Post(R"(/api/forum/post/(\d+)/media)", handle_upload_post_media);
    server.Get("/api/forum/recommend", handle_get_recommend_posts);
    server.Get("/api/tags", handle_get_tags);
    server.Post(R"(/api/forum/post/(\d+)/tags)", handle_add_tags_to_post);

    server.Post(R"(/api/user/(\w+)/follow)", handle_follow_user);
    server.Delete(R"(/api/user/(\w+)/unfollow)", handle_unfollow_user);
    server.Post(R"(/api/forum/post/(\d+)/like)", handle_like_post);
    server.Delete(R"(/api/forum/post/(\d+)/like)", handle_unlike_post);

    server.Get("/api/message/inbox", handle_get_messages);
    server.Post("/api/message/send", handle_send_message);

    server.Get(R"(/api/forum/post/(\d+)/comments)", handle_get_comments);
    server.Post(R"(/api/forum/post/(\d+)/comments)", handle_post_comment);
    server.Put(R"(/api/forum/post/(\d+)/comment/(\d+))", handle_edit_comment);
    server.Delete(R"(/api/forum/post/(\d+)/comment/(\d+))", handle_delete_comment);
    server.Post(R"(/api/forum/post/(\d+)/comment/(\d+)/media)", handle_upload_comment_media);

    server.Get("/api/draft", handle_get_drafts);
    server.Post("/api/draft", handle_create_draft);
    server.Put("/api/draft", handle_edit_draft);
    server.Delete("/api/draft", handle_delete_draft);
    server.Post("/api/draft/media", handle_upload_draft_media);
    server.Post("/api/draft/publish", handle_publish_draft);

    std::cout << "Server running at http://0.0.0.0:8888" << std::endl;

    server.listen("0.0.0.0", 8888);

    return 0;
}
