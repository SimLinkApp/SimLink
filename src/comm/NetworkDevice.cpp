

#include <spdlog/spdlog.h>

#include <string>
#include <thread>
#include <chrono>
#include <exception>

#include "comm/NetworkDevice.h"

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#include "comm/CommManager.h"

namespace simlink
{
    namespace communication_manager
    {

        /**
        * Constructor. Accepts a COM port name and a baud rate
        */
        NetworkDevice::NetworkDevice(boost::asio::io_service *io, std::string address, std::string descriptive_name)
        {
            // Store Asio io service
            this->io = io;

            this->device_address = address;
            this->device_name = descriptive_name;
            this->device_type = DEVICE_NETWORK;

            // Create new Socket object
            this->socket = new boost::asio::ip::tcp::socket(*(this->io));
        }

        /**
         * Connect to the device
         */
        void NetworkDevice::connect()
        {
            bool connected = false;
            while (!connected)
            {
                try
                {
                    // Run a connect
                    this->socket->connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(this->device_address), 5465));
                }
                catch (const boost::system::system_error &e)
                {
                    spdlog::error("Got error when trying to connect to device at {}: {}", this->device_address, e.code().value());
                }
                spdlog::info("Is Connected ({}): {}", this->device_address, this->socket->is_open());

                // Store the status of the socket
                connected = this->socket->is_open();

                // Sleep if it didn't connect
                if (!connected)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
            // Start reading on the socket
            this->run_async_read_until();
        }

        /**
        * Store the callback to be used later
        */
        void NetworkDevice::register_callback(device_callback _callback)
        {
            this->callback = _callback;
        }

        /**
        * Write data to the socket
        */
        int NetworkDevice::write_data(std::string data)
        {

            // Add the delimiter
            data.append("\n");

            spdlog::debug("Sending {} chars", data.length());

            // Convert it to a c string
            const char *data_c_str = data.c_str();

            spdlog::info("Sending network event: {}", data);

            try
            {
                // Send the data over the socket object (which we created earlier)
                size_t ret = boost::asio::write(*(this->socket), boost::asio::buffer(data_c_str, data.size()));
                spdlog::debug("Bytes written: {}", ret);
            }
            catch (const boost::system::system_error &e)
            {
                spdlog::error("Got error when trying to write network event to {}: {}", this->device_address, e.code().value());
            }

            return data.length();
        }

        void NetworkDevice::read_handler(const boost::system::error_code &error, std::size_t bytes_transferred)
        {
            spdlog::debug("Read {} bytes", bytes_transferred);

            // Prepare bytes to be transferred
            streambuf.prepare(bytes_transferred);

            // Use C++ magic to read from the port
            std::string cmd{
                buffers_begin(streambuf.data()),
                buffers_begin(streambuf.data()) + bytes_transferred - 1};

            // Run the callback
            this->callback(cmd);

            // Consume the bytes
            streambuf.consume(bytes_transferred);

            if (this->socket->is_open())
            {
                // Read it again
                this->run_async_read_until();
            }
        }

        void NetworkDevice::run_async_read_until()
        {
            boost::asio::async_read_until(
                *(this->socket),
                streambuf,
                '\n',
                boost::bind(
                    &NetworkDevice::read_handler,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        }

        void NetworkDevice::close()
        {
            this->socket->close();
        }

    } // namespace communication_manager
} // namespace simlink