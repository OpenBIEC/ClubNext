#ifndef ROUTES_SOCIAL_HPP
#define ROUTES_SOCIAL_HPP

#include <httplib.h>

void handle_follow_user(const httplib::Request &req, httplib::Response &res);
void handle_unfollow_user(const httplib::Request &req, httplib::Response &res);
void handle_like_post(const httplib::Request &req, httplib::Response &res);
void handle_unlike_post(const httplib::Request &req, httplib::Response &res);

#endif // SOCIAL_API_HPP
