#pragma once

#include <thread>
#include <memory>

namespace http {
namespace server {
class server;
struct request;
struct reply;
}
}

namespace mb {

class Application;

class REST_Server
{
private:
    std::thread m_thread;

    Application *m_application { nullptr };

    void exec();

    std::shared_ptr<::http::server::server> m_server;

    bool handle_index(const http::server::request &req, http::server::reply &rep);
    bool handle_lineup(const http::server::request &req, http::server::reply &rep);

    void setup();
public:
    REST_Server(Application *_application);
    ~REST_Server();

    void start();
    void stop();

    void start_async();
    void poll_one();
};

}
