#ifndef ROUTES_USER_HPP
#define ROUTES_USER_HPP

#include <httplib.h>

void handle_user_register(const httplib::Request &req, httplib::Response &res);
void handle_user_login(const httplib::Request &req, httplib::Response &res);
void handle_user_profile(const httplib::Request &req, httplib::Response &res);
void handle_user_update_avatar(const httplib::Request &req, httplib::Response &res);
void handle_user_get_profile(const httplib::Request &req, httplib::Response &res);

#endif // ROUTES_USER_HPP
