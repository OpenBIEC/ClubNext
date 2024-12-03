#ifndef ROUTES_FORUM_HPP
#define ROUTES_FORUM_HPP

#include <httplib.h>

void get_post_detail(const httplib::Request &req, httplib::Response &res);
void create_post(const httplib::Request &req, httplib::Response &res);
void upload_media(const httplib::Request &req, httplib::Response &res);
void get_recommend_posts(const httplib::Request &req, httplib::Response &res);
void get_tags(const httplib::Request &req, httplib::Response &res);
void add_tags_to_post(const httplib::Request &req, httplib::Response &res);

#endif // ROUTES_FORUM_HPP
