#include "send_verification_code.hpp"
#include "config.hpp"
#include <cstring>
#include <curl/curl.h>
#include <iostream>
#include <string>

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)
{
    const char **upload_data = static_cast<const char **>(userp);
    if (*upload_data)
    {
        size_t buffer_size = size * nmemb;
        size_t upload_len = strlen(*upload_data);

        if (upload_len > buffer_size)
        {
            memcpy(ptr, *upload_data, buffer_size);
            *upload_data += buffer_size;
            return buffer_size;
        }
        else
        {
            memcpy(ptr, *upload_data, upload_len);
            *upload_data = nullptr;
            return upload_len;
        }
    }
    return 0;
}

bool send_verification_code(const std::string &to_email, const std::string &code)
{
    const char *from_email = Config::VERIFY_EMAIL.c_str();
    const char *from_password = Config::VERIFY_PASSWORD.c_str();
    const char *smtp_server = Config::VERIFY_EMAIL_SERVER.c_str();

    std::string email_content =
        "From: \"TabVerify\" <" + std::string(from_email) +
        ">\r\n"
        "To: <" +
        to_email +
        ">\r\n"
        "Subject: Your Verification Code\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "\r\n"
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "  <style>"
        "    .container {"
        "      font-family: Arial, sans-serif;"
        "      margin: 20px;"
        "      padding: 20px;"
        "      border: 1px solid #ddd;"
        "      border-radius: 8px;"
        "      background-color: #f9f9f9;"
        "    }"
        "    .header {"
        "      font-size: 24px;"
        "      font-weight: bold;"
        "      color: #333;"
        "    }"
        "    .code {"
        "      font-size: 48px;"
        "      font-weight: bold;"
        "      color: #007bff;"
        "      text-align: center;"
        "      margin: 20px 0;"
        "    }"
        "    .footer {"
        "      font-size: 14px;"
        "      color: #666;"
        "    }"
        "  </style>"
        "</head>"
        "<body>"
        "  <div class='container'>"
        "    <div class='header'>Your Verification Code</div>"
        "    <div class='code'>" +
        code +
        "</div>"
        "    <div class='footer'>If you did not request this code, please ignore this email.</div>"
        "  </div>"
        "</body>"
        "</html>";

    CURL *curl = curl_easy_init();
    if (curl)
    {
        struct curl_slist *recipients = nullptr;

        curl_easy_setopt(curl, CURLOPT_URL, smtp_server);
        curl_easy_setopt(curl, CURLOPT_USERNAME, from_email);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, from_password);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from_email);
        recipients = curl_slist_append(recipients, to_email.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        const char *email_data = email_content.c_str();
        curl_easy_setopt(curl, CURLOPT_READDATA, &email_data);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        CURLcode res = curl_easy_perform(curl);
        bool success = (res == CURLE_OK);
        if (!success)
        {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
        }

        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);

        return success;
    }
    else
    {
        std::cerr << "Failed to initialize cURL" << std::endl;
        return false;
    }
}
