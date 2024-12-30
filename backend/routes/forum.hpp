#ifndef ROUTES_FORUM_HPP
#define ROUTES_FORUM_HPP

#include <httplib.h>

void handle_get_post_detail(const httplib::Request &req, httplib::Response &res);
void handle_create_post(const httplib::Request &req, httplib::Response &res);
void handle_modify_post(const httplib::Request &req, httplib::Response &res);
void handle_upload_post_media(const httplib::Request &req, httplib::Response &res);
void handle_get_recommend_posts(const httplib::Request &req, httplib::Response &res);
void handle_get_tags(const httplib::Request &req, httplib::Response &res);
void handle_add_tags_to_post(const httplib::Request &req, httplib::Response &res);

#endif // ROUTES_FORUM_HPP
