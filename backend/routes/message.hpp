#ifndef ROUTES_MESSAGE_HPP
#define ROUTES_MESSAGE_HPP

#include <httplib.h>

void handle_get_messages(const httplib::Request &req, httplib::Response &res);
void handle_send_message(const httplib::Request &req, httplib::Response &res);

#endif // ROUTES_MESSAGE_HPP
