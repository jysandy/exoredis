#include <memory>
#include <utility>
#include <functional>
#include <set>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
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
        expiry_timer_.expires_from_now(boost::posix_time::seconds(2));
        expiry_timer_.async_wait(std::bind(&db_session::handle_timer,
            this, asio::placeholders::error));
        do_accept();
    }

    void stop()
    {
        expiry_timer_.cancel();
        for (auto session: session_set)
        {
            session->stop();
        }
        session_set_.clear();
        db_.save();
    }

private:
    // Accept a connection and start a session.
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

    // Expires the database keys.
    void handle_timer(boost::system::error_code ec)
    {
        db_.expire_keys();
        expiry_timer_.expires_from_now(boost::posix_time::seconds(2));
        expiry_timer_.async_wait(std::bind(&exoredis_server::handle_timer,
            this, asio::placeholders::error));
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    exostore db_;
    std::set<db_session::pointer> session_set_;
    asio::deadline_timer expiry_timer_;
};
