#include "models/session_store.hpp"
#include "models/user_store.hpp"
#include "routes/user.hpp"
#include <chrono>
#include <csignal>
#include <httplib.h>
#include <iostream>
#include <thread>

SessionStore session_store;

void session_cleanup_task()
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::minutes(10)); // 每 10 分钟清理一次
        session_store.cleanup_sessions();
    }
}

UserStore user_store("user_store.json");

void signal_handler(int signal)
{
    if (signal == SIGINT)
    {
        std::cout << "\nReceived SIGINT, saving user data to file..." << std::endl;
        user_store.save_to_file();
        std::cout << "Data saved successfully. Exiting..." << std::endl;
        exit(0);
    }
}

int main()
{
    std::signal(SIGINT, signal_handler);

    std::thread cleanup_thread(session_cleanup_task);
    cleanup_thread.detach();

    httplib::Server server;

    server.Post("/api/user/register", handle_user_register);
    server.Post("/api/user/login", handle_user_login);
    server.Get("/api/user/profile", handle_user_profile);
    server.Post("/api/user/profile/update", handle_user_update_profile);
    server.Post("/api/user/avatar", handle_user_update_avatar);
    server.Get(R"(/api/user/(\w+)/profile)", handle_user_get_profile);

    std::cout << "Server running at http://0.0.0.0:8888" << std::endl;

    server.listen("0.0.0.0", 8888);

    return 0;
}
