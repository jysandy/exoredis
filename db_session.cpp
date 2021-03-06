#include "db_session.hpp"
#include "util.hpp"
#include <vector>
#include <functional>
#include <iostream>
#include <sstream>
#include <chrono>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/clamp.hpp>
#include <boost/bind.hpp>

namespace asio = boost::asio;

db_session::db_session(tcp::socket socket, exostore& db,
    std::set<db_session::pointer>& session_set)
    : socket_(std::move(socket)), db_(db), session_set_(session_set),
      out_stream_(&(this->write_buffer_))
{
}

void db_session::start()
{
    asio::async_read_until(socket_, read_buffer_, "\r\n",
        boost::bind(&db_session::handle_command_line, shared_from_this(),
            asio::placeholders::error, asio::placeholders::bytes_transferred));
}

void db_session::stop()
{
    socket_.close();
    session_set_.erase(shared_from_this());
}

// Parses the command and calls it.
void db_session::handle_command_line(const boost::system::error_code& ec,
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
            std::vector<unsigned char>> tok(command_string, sep);
        db_session::token_list command_tokens;
        try
        {
            command_tokens = db_session::token_list(
                tok.begin(), tok.end()
            );
        }
        catch (std::exception& e)
        {
            error_custom(std::string("Tokenizing error: ") + e.what());
            return;
        }

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
    asio::async_write(socket_, write_buffer_,
        boost::bind(&db_session::handle_write, shared_from_this(),
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
void db_session::call(const db_session::token_list& command_tokens)
{
    // Stringify the first token, which is the command name.
    std::string command_name = toupper_string(vec_to_string(command_tokens[0]));

    // Dispatch on command name.
    if (command_name == "GET")
    {
        get_command(command_tokens);
    }
    else if (command_name == "SET")
    {
        set_command(command_tokens);
    }
    else if (command_name == "GETBIT")
    {
        getbit_command(command_tokens);
    }
    else if (command_name == "SETBIT")
    {
        setbit_command(command_tokens);
    }
    else if (command_name == "ZADD")
    {
        zadd_command(command_tokens);
    }
    else if (command_name == "ZCARD")
    {
        zcard_command(command_tokens);
    }
    else if (command_name == "ZCOUNT")
    {
        zcount_command(command_tokens);
    }
    else if (command_name == "ZRANGE")
    {
        zrange_command(command_tokens);
    }
    else if (command_name == "SAVE")
    {
        save_command(command_tokens);
    }
    else
    {
        error_unknown_command(command_name);
    }
}

/*************
 * COMMANDS
 *************/

void db_session::get_command(const db_session::token_list& args)
{
    if (args.size() != 2)
    {
        error_incorrect_number_of_args("GET");
        return;
    }

    auto& key = args[1];

    // Throwing exceptions is expensive. Better to check using a bool if we can.
    if (!db_.key_exists(key))
    {
        write_nullbulk();
        return;
    }

    try
    {
        write_bstring(db_.get<exostore::bstring>(key));
    }
    catch (const exostore::key_error&)
    {
        write_nullbulk(); // Key might have expired after line 164 >_<
    }
    catch (const exostore::type_error&)
    {
        error_incorrect_type();
    }
}

void db_session::set_command(const db_session::token_list& args)
{
    if (args.size() > 5 || args.size() < 3)
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
    if (args.size() > 3)
    {
        try
        {
            for (auto it = args.begin() + 3; it != args.end(); it++)
            {
                auto option = toupper_string(vec_to_string(*it));
                if (option == "EX")
                {
                    seconds = boost::lexical_cast<long long>(
                        vec_to_string(*(++it)));
                    if (seconds <= 0)
                    {
                        error_syntax_error();
                        return;
                    }
                    ex_set = true;
                }
                else if (option == "PX")
                {
                    milliseconds = boost::lexical_cast<long long>(
                        vec_to_string(*(++it)));
                    if (milliseconds <= 0)
                    {
                        error_syntax_error();
                        return;
                    }
                    px_set = true;
                }
                else if (option == "XX")
                {
                    xx_set = true;
                }
                else if (option == "NX")
                {
                    nx_set = true;
                }
                else
                {
                    error_syntax_error();
                    return;
                }
            }
        }
        catch (const boost::bad_lexical_cast&)
        {
            error_syntax_error();
            return;
        }
    }

    if ((px_set && ex_set) || (nx_set && xx_set))
    {
        error_syntax_error();
        return;
    }

    auto& key = args[1];
    if ((db_.key_exists(key) && nx_set) || (!db_.key_exists(key) && xx_set))
    {
        write_nullbulk();
        return;
    }

    if (ex_set)
    {
        db_.set(key, exostore::bstring(args[2], 1000 * seconds));
    }
    else if (px_set)
    {
        db_.set(key, exostore::bstring(args[2], milliseconds));
    }
    else
    {
        db_.set(key, exostore::bstring(args[2]));
    }
    write_simple_string("OK");
}

void db_session::getbit_command(const db_session::token_list& args)
{
    if (args.size() != 3)
    {
        error_incorrect_number_of_args("GETBIT");
        return;
    }

    auto& key = args[1];
    if (!db_.key_exists(key))
    {
        write_integer(0);
        return;
    }

    try
    {
        auto& value = db_.get<exostore::bstring>(key);
        auto int_offset = boost::lexical_cast<long long>(
            vec_to_string(args[2])
        );

        if (int_offset < 0)
        {
            error_syntax_error();
            return;
        }

        auto byte_offset = int_offset / 8;
        int bit_offset_from_right = 7 - (int_offset % 8);

        if ((byte_offset) >= value.bdata().size())
        {
            write_integer(0);
            return;
        }

        unsigned char byte_in_question = value.bdata()[byte_offset];
        // When bit shifting, we prefer left shift since the fill value for
        // right shift is not defined for signed data types.
        unsigned char bit_value = (byte_in_question << (7 - bit_offset_from_right))
            & static_cast<unsigned char>(0x80u);
        if (bit_value != 0)
        {
            write_integer(1);
            return;
        }
        else
        {
            write_integer(0);
            return;
        }
    }
    catch (const exostore::key_error&)
    {
        write_integer(0);
        return;
    }
    catch (const exostore::type_error&)
    {
        error_incorrect_type();
        return;
    }
    catch (const boost::bad_lexical_cast&)
    {
        error_syntax_error();
        return;
    }
}

void db_session::setbit_command(const db_session::token_list& args)
{
    if (args.size() != 4)
    {
        error_incorrect_number_of_args("SETBIT");
        return;
    }


    try
    {
        auto int_offset = boost::lexical_cast<long long>(
            vec_to_string(args[2])
        );

        if (int_offset < 0)
        {
            std::cout << "Offset error" << std::endl;
            error_syntax_error();
            return;
        }

        int bit_value = boost::lexical_cast<int>(
            vec_to_string(args[3])
        );

        if ((bit_value < 0) || (bit_value > 1))
        {
            error_syntax_error();
            return;
        }

        auto& key = args[1];
        if (!db_.key_exists(key))
        {
            db_.set(key, exostore::bstring());
        }
        auto& value = db_.get<exostore::bstring>(key);

        auto byte_offset = int_offset / 8;
        int bit_offset_from_right = 7 - (int_offset % 8);

        if (byte_offset >= value.bdata().size())
        {
            value.bdata().resize(byte_offset + 1, static_cast<unsigned char>(0));
        }

        unsigned char byte_in_question = value.bdata()[byte_offset];
        int return_value = (byte_in_question << (7 - bit_offset_from_right))
            & static_cast<unsigned char>(0x80u);

        if (return_value != 0)
        {
            return_value = 1;
        }

        if (bit_value == 1)
        {
            byte_in_question |= static_cast<unsigned char>(1u) << bit_offset_from_right;
        }
        else
        {
            byte_in_question &= ~(static_cast<unsigned char>(1u) << bit_offset_from_right);
        }

        value.bdata()[byte_offset] = byte_in_question;

        write_integer(return_value);
    }
    catch (const exostore::key_error&)  // This shouldn't happen, I set the key earlier
    {
        error_key_does_not_exist();
        return;
    }
    catch (const exostore::type_error&)
    {
        error_incorrect_type();
        return;
    }
    catch (const boost::bad_lexical_cast&)
    {
        error_syntax_error();
        return;
    }
}

void db_session::zadd_command(const db_session::token_list& args)
{
    if (args.size() < 4)
    {
        error_incorrect_number_of_args("ZADD");
        return;
    }

    bool nx_set = false;
    bool xx_set = false;
    bool ch_set = false;
    bool incr_set = false;

    // Parse the command and set flags.
    if (args.size() > 4)    // There aren't any flags otherwise
    {
        for (auto it = args.begin() + 2; it != args.end() - 2; it++)
        {
            auto option = toupper_string(vec_to_string(*it));
            if (option == "NX")
            {
                nx_set = true;
            }
            else if (option == "XX")
            {
                xx_set = true;
            }
            else if (option == "CH")
            {
                ch_set = true;
            }
            else if (option == "INCR")
            {
                incr_set = true;
            }
            else
            {
                error_syntax_error();
            }
        }
    }

    if (nx_set && xx_set)
    {
        error_syntax_error();
        return;
    }

    // Get the score/member pair.
    auto it = args.end() - 2;
    auto score_bstring = *it;
    it++;
    auto& member = *it;

    double score = 0.0;
    try
    {
        score = boost::lexical_cast<double>(
            vec_to_string(score_bstring)
        );
    }
    catch (const boost::bad_lexical_cast&)
    {
        error_syntax_error();
        return;
    }

    int return_value = 0;
    auto& key = args[1];

    // Create the sorted set if key doesn't exist.
    if (!db_.key_exists(key))
    {
        if (xx_set)
        {
            // Nothing to do here. Bail out early without creating a set.
            write_integer(0);
            return;
        }
        db_.set(key, exostore::zset());
    }

    try
    {
        auto& accessed_set = db_.get<exostore::zset>(key);

        if (nx_set && accessed_set.contains(member)
            || xx_set && !accessed_set.contains(member))
        {
            write_integer(0);
            return;
        }

        if (incr_set)
        {
            // Doesn't matter if ch is set or not in this case.
            double current_score = accessed_set.get_score(member);
            double new_score = current_score + score;
            accessed_set.add(member, new_score);
            write_bstring(boost::lexical_cast<std::string>(new_score));
            return;
        }
        else
        {
            if (accessed_set.contains_element_score(member, score))
            {
                // Nothing added or changed.
                write_integer(0);
                return;
            }

            if (accessed_set.contains(member) && !ch_set)
            {
                accessed_set.add(member, score);
                write_integer(0);
                return;
            }
            else
            {
                /*
                Either the member is contained and ch is set (write 1),
                or the member is not contained (write 1 regardless
                of ch).
                */
                accessed_set.add(member, score);
                write_integer(1);
                return;
            }
        }
    }
    catch (const exostore::type_error&)
    {
        error_incorrect_type();
        return;
    }
}

void db_session::zcard_command(const db_session::token_list& args)
{
    if (args.size() != 2)
    {
        error_incorrect_number_of_args("ZCARD");
        return;
    }

    try
    {
        auto& accessed_set = db_.get<exostore::zset>(args[1]);
        write_integer(accessed_set.size());
    }
    catch (const exostore::key_error& )
    {
        write_integer(0);
    }
    catch (const exostore::type_error& )
    {
        error_incorrect_type();
    }
}

void db_session::zcount_command(const db_session::token_list& args)
{
    if (args.size() != 4)
    {
        error_incorrect_number_of_args("ZCOUNT");
        return;
    }

    auto& key = args[1];
    try
    {
        double min = boost::lexical_cast<double>(
            vec_to_string(args[2])
        );
        double max = boost::lexical_cast<double>(
            vec_to_string(args[3])
        );
        auto& accessed_set = db_.get<exostore::zset>(key);
        write_integer(accessed_set.count(min, max));
    }
    catch (const boost::bad_lexical_cast& )
    {
        error_syntax_error();
    }
    catch (const exostore::type_error& )
    {
        error_incorrect_type();
    }
}

void db_session::zrange_command(const db_session::token_list& args)
{
    if (args.size() < 4 || args.size() > 5)
    {
        error_incorrect_number_of_args("ZRANGE");
        return;
    }

    bool withscores = false;
    if (args.size() == 5
        && toupper_string(vec_to_string(args[4])) == "WITHSCORES")
    {
        withscores = true;
    }

    auto& key = args[1];
    if (!db_.key_exists(key))
    {
        // Write an empty array.
        write_array(std::vector<std::vector<unsigned char>>());
        return;
    }

    try
    {
        auto& accessed_set = db_.get<exostore::zset>(key);
        auto start = boost::lexical_cast<long long>(
            vec_to_string(args[2])
        );
        auto end = boost::lexical_cast<long long>(
            vec_to_string(args[3])
        );

        // Convert negative args to zero-based offsets from the start.
        auto set_length = accessed_set.size();
        if (start < 0)
        {
            start = set_length + start;
        }

        if (end < 0)
        {
            end = set_length + end;
        }

        // Clamp both.
        start = boost::algorithm::clamp(start, 0, set_length - 1);
        end = boost::algorithm::clamp(end, 0, set_length - 1);

        if (start > end)
        {
            write_array(std::vector<std::vector<unsigned char>>());
            return;
        }

        auto iterators = accessed_set.element_range(start, end);
        std::vector<std::vector<unsigned char>> array_output;

        for (auto it = iterators.first; it != iterators.second;
            it++)
        {
            array_output.emplace_back(it->member());
            if (withscores)
            {
                array_output.emplace_back(string_to_vec(
                    boost::lexical_cast<std::string>(it->score())));
            }
        }

        write_array(array_output);
    }
    catch (const exostore::type_error&)
    {
        error_incorrect_type();
    }
    catch (const boost::bad_lexical_cast&)
    {
        error_syntax_error();
    }
}

void db_session::save_command(const db_session::token_list& args)
{
    if (args.size() != 1)
    {
        error_incorrect_number_of_args("SAVE");
    }

    db_.save();
    write_simple_string("OK");
}

/******************
 * RESPONSES
 ******************/

// Writes out binary data as a bulk string.
void db_session::write_bstring(const exostore::bstring& bstr)
{
    write_bstring(bstr.bdata());
}

void db_session::write_bstring(const std::string& str)
{
    write_bstring(string_to_vec(str));
}

void db_session::write_bstring(const std::vector<unsigned char>& bdata)
{
    out_stream_ << '$' << bdata.size() << "\r\n";
    out_stream_.write(reinterpret_cast<const char*>(bdata.data()), bdata.size());
    out_stream_ << "\r\n" << std::flush;
    do_write();
}

void db_session::write_nullbulk()
{
    out_stream_ << "$-1\r\n" << std::flush;
    do_write();
}

void db_session::write_simple_string(const std::string& str)
{
    out_stream_ << "+" << str << "\r\n" << std::flush;
    do_write();
}

void db_session::write_integer(const long long& integer)
{
    out_stream_ << ":" << integer << "\r\n" << std::flush;
    do_write();
}

void db_session::write_array(const std::vector<std::vector<unsigned char>>&
    array_to_write)
{
    out_stream_ << "*" << array_to_write.size() << "\r\n";
    for (auto& bdata: array_to_write)
    {
        out_stream_ << '$' << bdata.size() << "\r\n";
        out_stream_.write(reinterpret_cast<const char*>(bdata.data()), bdata.size());
        out_stream_ << "\r\n";
    }
    out_stream_ << std::flush;
    do_write();
}

/*****************
 * ERROR MESSAGES
 *****************/

void db_session::error_unknown_command(std::string command_name)
{
    out_stream_ << "-ERR Unknown command " << command_name << "\r\n"
        << std::flush;
    do_write();
}

void db_session::error_incorrect_number_of_args(std::string command_name)
{
    out_stream_ << "-ERR Incorrect number of args for " << command_name
        << "\r\n" << std::flush;
    do_write();
}

void db_session::error_key_does_not_exist()
{
    out_stream_ << "-ERR Key does not exist\r\n";
    do_write();
}

void db_session::error_incorrect_type()
{
    out_stream_ << "-ERR Incorrect type\r\n";
    do_write();
}

void db_session::error_syntax_error()
{
    out_stream_ << "-ERR Syntax error\r\n";
    do_write();
}

void db_session::error_custom(const std::string& msg)
{
    out_stream_ << "-ERR " << msg << "\r\n";
    do_write();
}
