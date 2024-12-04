#ifndef ROUTES_COMMENT_HPP
#define ROUTES_COMMENT_HPP

#include <httplib.h>

void handle_get_comments(const httplib::Request &req, httplib::Response &res);
void handle_post_comment(const httplib::Request &req, httplib::Response &res);
void handle_edit_comment(const httplib::Request &req, httplib::Response &res);
void handle_delete_comment(const httplib::Request &req, httplib::Response &res);
void handle_upload_comment_media(const httplib::Request &req, httplib::Response &res);

#endif // ROUTES_COMMENT_HPP
