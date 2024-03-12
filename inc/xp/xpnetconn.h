

#ifndef _XP_NETCONN_H
#define _XP_NETCONN_H

#include <string>

#include <boost/asio.hpp>

namespace simlink
{
    namespace xp
    {
        void connect(std::string host);
        void disconnect();
        void send_message(std::string message);
        void handle_read_event(std::string message);
        void read_handler(const boost::system::error_code &error, std::size_t bytes_transferred);
        void run_async_read_until();
    } // namespace xp
} // namespace simlink

#endif