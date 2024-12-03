#ifndef AUTHENTICATE_HPP
#define AUTHENTICATE_HPP

#include <httplib.h>
#include <string>

bool authenticate_user(const httplib::Request &req, std::string &username);

#endif // AUTHENTICATE_HPP
