#include "mb_rest_server.h"
#include "http/server/server.hpp"
#include "http/server/request.hpp"
#include "http/server/reply.hpp"

#include "common/mb_types.h"
#include "common/mb_globals.h"

#include "mb_application.h"

#include <functional>

namespace {

const char *open_api { R"openapi(openapi: 3.0.0
info:
    version: 1.0.0
    title: MidiaBox API
    description: API do MidiaBox

servers:
    - url: http://192.168.70.10

paths:
    /list:
        get:
            description: Returns a list of stuff
            responses:
                '200':
                    description: Successful response
)openapi" };

}

namespace mb {

REST_Server::REST_Server(Application *_application):
    m_application(_application)
{
}

REST_Server::~REST_Server()
{
    stop();
}

void REST_Server::start()
{
    m_thread = std::thread(std::bind(&REST_Server::exec, this));
}

void REST_Server::stop()
{
    if(m_server)
    {
        m_server->stop();
        m_server.reset();
        m_thread.join();
    }
}

void REST_Server::start_async()
{
    setup();
}

void REST_Server::poll_one()
{
    if(m_server)
        m_server->poll_one();
}

void REST_Server::setup()
{
    m_server = std::make_shared<http::server::server>("0.0.0.0", "80", "/usr/local/var/html");
    auto server { m_server };

    using namespace std::placeholders;

    //server->add_endpoint("/", std::bind(&REST_Server::handle_index, this, _1, _2));
    server->add_endpoint("/lineup", std::bind(&REST_Server::handle_lineup, this, _1, _2));
}

void REST_Server::exec()
{
    setup();
    m_server->run();
}

bool REST_Server::handle_index(const http::server::request& /*req*/, http::server::reply& rep)
{
    rep.status = http::server::reply::ok;
    rep.content = open_api;
    rep.headers.emplace_back("content-type", "text/plain; charset=utf-8");
    return true;
}

bool REST_Server::handle_lineup(const http::server::request& /*req*/, http::server::reply& rep)
{
    rep.status = http::server::reply::ok;

    const auto& line_up { m_application->get_lineup() };

    rep.content = serialize(line_up);
    rep.headers.emplace_back("content-type", "application/json; charset=utf-8");

    return true;
}

}

#include <boost/throw_exception.hpp>
#ifdef BOOST_NO_EXCEPTIONS

namespace boost
{

BOOST_NORETURN void throw_exception( std::exception const & e ) // user defined
{
    DEBUG_MSG("Exception: " << e.what() << endl);
    exit(0);
}

BOOST_NORETURN void throw_exception( std::exception const & e, boost::source_location const & loc ) // user defined
{
    DEBUG_MSG("Exception: " << loc.to_string() << " -> "  << e.what() << endl);
    exit(0);
}

}

#endif // BOOST_NO_EXCEPTIONS
