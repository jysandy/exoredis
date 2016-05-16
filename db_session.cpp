#include "db_session.hpp"
#include "util.hpp"
#include <vector>
#include <functional>
#include <iostream>
#include <sstream>
#include <chrono>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace asio = boost::asio;

db_session::db_session(tcp::socket socket, exostore& db,
    std::set<db_session::pointer>& session_set, asio::io_service& io)
    : socket_(std::move(socket), db_(db), session_set_(session_set)),
    expiry_timer_(io)
{
    expiry_timer_.expires_from_now(boost::posix_time::seconds(2));
    expiry_timer_.async_wait(std::bind(&db_session::handle_timer,
        shared_from_this(), asio::placeholders::error));
}

// Read a line of data..
void db_session::start()
{
    asio::async_read_until(socket_, read_buffer_, "\r\n",
        std::bind(&db_session::handle_command_line, shared_from_this(),
            asio::placeholders::error, asio::placeholders::bytes_transferred))
}

// Shuts down and removes this session from the pool.
void db_session::stop()
{
    socket_->close();
    session_set_.erase(shared_from_this());
}

// Expires the database keys.
void db_session::handle_timer(boost::system::error_code ec)
{
    db_.expire_keys();
    expiry_timer_.expires_from_now(boost::posix_time::seconds(2));
    expiry_timer_.async_wait(std::bind(&db_session::handle_timer,
        shared_from_this(), asio::placeholders::error));
}

// Parses the command and calls it.
void db_session::handle_command_line(boost::system::error_code ec,
    std::size_t bytes_transferred)
{
    if (!ec)
    {
        auto bufs = read_buffer_.data();
        std::vector<unsigned char> command_string(
            asio::buffers_begin(bufs),
            asio::buffers_begin(bufs) + (bytes_transferred - 2) // Don't want the \r\n
        );
        read_buffer_.consume(bytes_transferred);

        boost::escaped_list_separator<unsigned char> sep('\\', ' ', '\"');
        boost::tokenizer<
            boost::escaped_list_separator<unsigned char>,
            std::vector<unsigned char>::const_iterator,
            std::vector<unsigned char>> tok(v, sep);
        std::vector<std::vector<unsigned char>> command_tokens(
            tok.begin(), tok.end());
        call(command_tokens);
    }
    else
    {
        stop();
    }
}

// Writes the response...
void db_session::do_write()
{
    asio::async_write(socket_, asio::buffer(write_buffer_),
        std::bind(&db_session::handle_write, shared_from_this(),
            asio::placeholders::error));
}

// ...then starts reading again.
void db_session::handle_write(boost::system::error_code ec)
{
    if (!ec)
    {
        start();
    }
    else
    {
        stop();
    }
}

// Calls the appropriate command, or writes out an error response if the
// format is invalid.
void db_session::call(db_session::token_list command_tokens)
{
    // Stringify the first token, which is the command name.
    std::string command_name = vec_to_string(command_tokens[0]);

    db_session::token_list command_args(
        command_tokens.begin() + 1,
        command_tokens.end()
    );

    // Dispatch on command name.
    switch (command_name)
    {
        case "GET":
            get_command(command_args);
            break;
        case "SET":
            set_command(command_args);
            break;
        case "GETBIT":
            getbit_command(command_args);
            break;
        case "SETBIT":
            setbit_command(command_args);
            break;
        case "ZADD":
            zadd_command(command_args);
            break;
        case "ZCARD":
            zcard_command(command_args);
            break;
        case "ZRANGE":
            zrange_command(command_args);
            break;
        case "SAVE":
            save_command(command_args);
            break;
        default:
            error_unknown_command(command_name);
            break;
    }
}



void db_session::get_command(db_session::token_list args)
{
    if (args.size() != 1)
    {
        error_incorrect_number_of_args("GET");
        return;
    }

    auto& key = args[0];

    // Throwing exceptions is expensive. Better to check using a bool if we can.
    if (!db_.key_exists(key))
    {
        error_key_does_not_exist();
    }

    try
    {
        write_bstring(db_.get<exostore::bstring>(key));
    }
    catch(exostore::key_error&)
    {
        error_key_does_not_exist(); // Key might have expired after line 155 >_<
    }
    catch (exostore::type_error&)
    {
        error_incorrect_type();
    }
}

void db_session::set_command(db_session::token_list args)
{
    if (args.size() > 4)
    {
        error_incorrect_number_of_args("SET");
        return;
    }

    bool ex_set = false;
    bool px_set = false;
    bool nx_set = false;
    bool xx_set = false;
    long long milliseconds = 0;
    long long seconds = 0;

    // Parse the command and set flags.
    try
    {
        for (auto it = args.begin() + 3; it != args.end(); it++)
        {
            switch (vec_to_string(*it))
            {
                case "EX":
                    if (px_set)
                    {
                        error_syntax_error();
                        return;
                    }
                    seconds = boost::lexical_cast<unsigned long long>(
                        vec_to_string(*(++it)));
                    ex_set = true;
                    break;

                case "PX":
                    if (ex_set)
                    {
                        error_syntax_error();
                        return;
                    }
                    milliseconds = boost::lexical_cast<unsigned long long>(
                        vec_to_string(*(++it)));
                    px_set = true;
                    break;

                case "XX":
                    if (nx_set)
                    {
                        error_syntax_error();
                        return;
                    }
                    xx_set = true;
                    break;

                case "NX":
                    if (xx_set)
                    {
                        error_syntax_error();
                        return;
                    }
                    nx_set = true;
                    break;
            }
        }
    }
    catch (const boost::bad_lexical_cast&)
    {
        error_syntax_error();
        return;
    }

    auto& key = args[0];
    if ((db.key_exists(key) && nx_set) || (!db.key_exists(key) && xx_set))
    {
        write_nullbulk();
        return;
    }

    if (ex_set)
    {
        db_.set(key, exostore::bstring(args[1], 1000 * seconds));
    }
    else if (px_set)
    {
        db_.set(key, exostore::bstring(args[1], milliseconds));
    }
    else
    {
        db_.set(key, exostore::bstring(args[1]));
    }
    write_simple_string("OK");
}
