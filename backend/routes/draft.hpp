#ifndef ROUTES_DRAFT_HPP
#define ROUTES_DRAFT_HPP

#include <httplib.h>

void handle_get_drafts(const httplib::Request &req, httplib::Response &res);
void handle_create_draft(const httplib::Request &req, httplib::Response &res);
void handle_edit_draft(const httplib::Request &req, httplib::Response &res);
void handle_delete_draft(const httplib::Request &req, httplib::Response &res);
void handle_upload_draft_media(const httplib::Request &req, httplib::Response &res);
void handle_publish_draft(const httplib::Request &req, httplib::Response &res);

#endif // ROUTES_DRAFT_HPP
