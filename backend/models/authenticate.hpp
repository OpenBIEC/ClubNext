#ifndef AUTHENTICATE_HPP
#define AUTHENTICATE_HPP

#include "models/session_store.hpp"
#include "models/user_store.hpp"
#include <httplib.h>
#include <string>

extern SessionStore session_store;
extern UserStore user_store;

bool authenticate_user(const httplib::Request &req, std::string &username);

#endif // AUTHENTICATE_HPP
