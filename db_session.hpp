#ifndef __EXOREDIS_DB_SESSION_HPP__
#define __EXOREDIS_DB_SESSION_HPP__


#include <memory>
#include <utility>
#include <set>
#include <vector>
#include <boost/asio.hpp>
#include "exostore.hpp"

namespace asio = boost::asio;
using boost::asio::ip::tcp;


class db_session
    : public std::enable_shared_from_this<db_session>
{
public:
    typedef std::shared_ptr<db_session> pointer;

    db_session(tcp::socket socket, exostore& db,
        std::set<db_session::pointer>& session_set, asio::io_service& io);

    void start();
    void stop();

private:
    typedef std::vector<std::vector<unsigned char>> token_list;

    void handle_timer(boost::system::error_code ec);

    void handle_command_line(boost::system::error_code ec, std::size_t bytes_transferred);
    void do_write();
    void handle_write(boost::system::error_code ec);
    void call(token_list command_tokens);

    // TODO all these
    void write_bstring(const exostore::bstring&);
    void write_bstring(const std::string&);
    void write_simple_string(const std::string&);
    void write_nullbulk();
    void write_integer(const long long&);


    // Commands
    // A command is responsible for calling do_write to write the response.
    void get_command(token_list args);
    void set_command(token_list args);
    void getbit_command(token_list args);
    void setbit_command(token_list args);
    // TODO all these
    void zadd_command(token_list args);
    void zcard_command(token_list args);
    void zrange_command(token_list args);
    void save_command(token_list args);

    // Errors
    // Write error messages as responses
    // TODO all these
    void error_unknown_command(std::string command_name);
    void error_incorrect_number_of_args(std::string command_name);
    void error_key_does_not_exist();
    void error_incorrect_type();
    void error_syntax_error();

    tcp::socket socket_;
    exostore& db_;
    std::set<db_session::pointer>& session_set_;
    asio::streambuf read_buffer_;
    asio::streambuf write_buffer_;
    std::vector<unsigned char> response_string_;
    asio::deadline_timer expiry_timer_;
};

#endif
