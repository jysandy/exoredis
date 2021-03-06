#ifndef __EXOREDIS_DB_SESSION_HPP__
#define __EXOREDIS_DB_SESSION_HPP__


#include <memory>
#include <utility>
#include <set>
#include <vector>
#include <boost/asio.hpp>
#include <cstddef>
#include "exostore.hpp"

namespace asio = boost::asio;
using boost::asio::ip::tcp;



/*
 * Represents a connection to the database.
 * Responsible for reading commands, parsing them, calling the DB API to run
 * the commands and writing the response.
 */
class db_session
    : public std::enable_shared_from_this<db_session>
{
public:
    typedef std::shared_ptr<db_session> pointer;

    db_session(tcp::socket socket, exostore& db,
        std::set<db_session::pointer>& session_set);

    // Starts reading data.
    void start();

    // Shuts down and removes this session from the pool.
    void stop();

private:
    typedef std::vector<std::vector<unsigned char>> token_list;

    void handle_command_line(const boost::system::error_code& ec,
        std::size_t bytes_transferred);

    // Writes out the contents of the write buffer.
    void do_write();
    void handle_write(boost::system::error_code ec);

    // Calls the appropriate command, given a list of tokens.
    void call(const token_list& command_tokens);

    // Responses
    void write_bstring(const exostore::bstring&);
    void write_bstring(const std::string&);
    void write_bstring(const std::vector<unsigned char>&);
    void write_nullbulk();
    void write_simple_string(const std::string&);
    void write_integer(const long long&);
    void write_array(const std::vector<std::vector<unsigned char>>&);


    // Commands
    // A command is responsible for calling do_write to write the response.
    void get_command(const token_list& args);
    void set_command(const token_list& args);
    void getbit_command(const token_list& args);
    void setbit_command(const token_list& args);
    void zadd_command(const token_list& args);
    void zcard_command(const token_list& args);
    void zcount_command(const token_list& args);
    void zrange_command(const token_list& args);
    void save_command(const token_list& args);

    // Errors
    // Write error messages as responses
    void error_unknown_command(std::string command_name);
    void error_incorrect_number_of_args(std::string command_name);
    void error_key_does_not_exist();
    void error_incorrect_type();
    void error_syntax_error();
    void error_custom(const std::string& msg);

    tcp::socket socket_;
    exostore& db_;
    std::set<db_session::pointer>& session_set_;
    asio::streambuf read_buffer_;
    asio::streambuf write_buffer_;
    std::ostream out_stream_;
};

#endif
