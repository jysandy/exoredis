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
        : acceptor_(io, endpoint), socket_(io), db_(db_path),
          signals_(io, SIGINT)
    {
        try
        {
            db_.load();
        }
        catch (const exostore::load_error& e)
        {
            std::cout << e.what << std::endl;
        }
        expiry_timer_.expires_from_now(boost::posix_time::seconds(2));
        expiry_timer_.async_wait(std::bind(&exoredis_server::handle_timer,
            this, asio::placeholders::error));
        signals_.async_wait(std::bind(&exoredis_server::handle_signal, this,
            asio::placeholders::error, asio::placeholders::signal_number));
        do_accept();
    }

    ~exoredis_server()
    {
    }

    void stop()
    {
        expiry_timer_.cancel();
        for (auto& session: session_set)
        {
            session.stop();
        }
        session_set_.clear();
        db_.save();
        acceptor_.close();
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

    void handle_signal(boost::system::error_code ec, int signal_number)
    {
        stop();
    }

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    exostore db_;
    std::set<db_session::pointer> session_set_;
    asio::deadline_timer expiry_timer_;
    asio::signal_set signals_;
};

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: exoredis <db_path>" << std::endl;
            return 1;
        }

        asio::io_service io_service;
        exoredis_server server(io_service, tcp::endpoint(tcp::v4(), 15000),
            argv[1]);
        server.start();
        io_service.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
