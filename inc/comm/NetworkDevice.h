

#ifndef _NETWORK_DEVICE_H
#define _NETWORK_DEVICE_H

#include <string>

#include "comm/CommManager.h"

#include "boost/asio.hpp"

namespace simlink
{
    namespace communication_manager
    {

        class NetworkDevice : public Device
        {
        public:
            NetworkDevice(boost::asio::io_service *io, std::string address, std::string descriptive_name);
            void connect();
            void register_callback(device_callback callback);
            int write_data(std::string data);
            void read_handler(const boost::system::error_code &error, std::size_t bytes_transferred);
            void run_async_read_until();
            void close();

        private:
            boost::asio::streambuf streambuf;
            device_callback callback;
            boost::asio::io_service *io;
            boost::asio::ip::tcp::socket *socket;
            NetworkDevice();
        };
    } // namespace communication_manager
} // namespace simlink
#endif