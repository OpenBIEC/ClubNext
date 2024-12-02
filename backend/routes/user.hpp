#ifndef ROUTES_USER_HPP
#define ROUTES_USER_HPP

#include "models/session_store.hpp"
#include "models/user_store.hpp"
#include <httplib.h>

extern SessionStore session_store;
extern UserStore user_store;

bool authenticate_user(const httplib::Request &req, std::string &username);

void handle_user_register(const httplib::Request &req, httplib::Response &res);
void handle_user_login(const httplib::Request &req, httplib::Response &res);
void handle_user_profile(const httplib::Request &req, httplib::Response &res);
void handle_user_update_profile(const httplib::Request &req, httplib::Response &res);
void handle_user_update_avatar(const httplib::Request &req, httplib::Response &res);
void handle_user_get_profile(const httplib::Request &req, httplib::Response &res);

#endif // ROUTES_USER_HPP
