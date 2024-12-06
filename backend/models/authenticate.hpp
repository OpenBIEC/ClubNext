#ifndef AUTHENTICATE_HPP
#define AUTHENTICATE_HPP

#include <httplib.h>
#include <string>

bool authenticate_user(const httplib::Request &req, std::string &username, int accept_mode = 0);
std::string generate_verification_code(size_t length = 6);

#endif // AUTHENTICATE_HPP
