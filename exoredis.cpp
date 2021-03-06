#include <memory>
#include <utility>
#include <set>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include "exostore.hpp"
#include "db_session.hpp"

namespace asio = boost::asio;
using boost::asio::ip::tcp;

/*
 * The fundamental server class. OWns the database.
 * Responsible for accepting and managing new connections.
 * Also runs a timer to expire keys from the database.
 */
class exoredis_server
{
public:
    exoredis_server(asio::io_service& io, const tcp::endpoint& endpoint,
        std::string db_path)
        : acceptor_(io, endpoint), socket_(io), db_(db_path),
          expiry_timer_(io), signals_(io, SIGINT), io_(io)
    {
        std::cout << "Starting server..." << std::endl;
        try
        {
            db_.load();
        }
        catch (const exostore::load_error& e)
        {
            std::cout << e.what() << std::endl;
        }
        expiry_timer_.expires_from_now(boost::posix_time::seconds(2));
        expiry_timer_.async_wait(boost::bind(&exoredis_server::handle_timer,
            this, asio::placeholders::error));
        signals_.async_wait(boost::bind(&exoredis_server::handle_signal, this,
            asio::placeholders::error, asio::placeholders::signal_number));
        do_accept();
        std::cout << "Server started." << std::endl;
    }

    ~exoredis_server()
    {
    }

    void stop()
    {
        std::cout << "\nStopping server..." << std::endl;
        expiry_timer_.cancel();
        for (auto session: session_set_)
        {
            session->stop();
        }
        session_set_.clear();
        db_.save();
        acceptor_.close();
        socket_.close();
        signals_.cancel();
        io_.stop();
        std::cout << "Server stopped." << std::endl;
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
                session_set_.insert(new_session);
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
        expiry_timer_.async_wait(boost::bind(&exoredis_server::handle_timer,
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
    asio::io_service& io_;
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
        io_service.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
