#include <httplib.h>
#include <iostream>
#include <nlohmann/json.hpp>

#include "routes/forum.hpp"
#include "routes/message.hpp"
#include "routes/user.hpp"

int main()
{

    httplib::Server server;

    std::cout << "Server is running on http://0.0.0.0:8080" << std::endl;
    server.listen("0.0.0.0", 8080);

    return 0;
}
