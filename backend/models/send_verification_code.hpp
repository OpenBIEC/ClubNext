#ifndef MODELS_SEND_VERIFICATION_CODE_HPP
#define MODELS_SEND_VERIFICATION_CODE_HPP

#include <string>

bool send_verification_code(const std::string &to_email, const std::string &code);

#endif