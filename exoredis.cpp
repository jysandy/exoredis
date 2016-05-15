#include <memory>
#include <utility>
#include <functional>
#include <set>
#include <boost/asio.hpp>
#include <iostream>
#include "exostore.hpp"
#include "db_session.hpp"

namespace asio = boost::asio;
using boost::asio::ip::tcp;


class exoredis_server
{
public:
    exoredis_server(asio::io_service& io, const tcp::endpoint& endpoint,
        std::string db_path)
        : acceptor_(io, endpoint), socket_(io), db_(db_path)
    {
        do_accept();
    }

    void stop()
    {
        for (auto session: session_set)
        {
            session->stop();
        }
        session_set_.clear();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(socket_,
        [this](boost::system::error_code ec)
        {
            if (!ec)
            {
                auto new_session = std::make_shared<db_session>(
                    std::move(socket_), db_, session_set_);
                session_set_.add(new_session);
                new_session->start();
            }

            do_accept();
        });
    }
    

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    exostore db_;
    std::set<db_session::pointer> session_set_;
};
