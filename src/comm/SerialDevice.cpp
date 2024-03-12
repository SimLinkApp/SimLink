

#include "spdlog/spdlog.h"

#include <string>
#include <thread>
#include <chrono>
#include <exception>

#include "boost/asio.hpp"
#include "boost/bind/bind.hpp"

#include "comm/CommManager.h"

#include "comm/SerialDevice.h"

namespace simlink
{
    namespace communication_manager
    {

        /**
        * Constructor. Accepts a COM port name and a baud rate
        */
        SerialDevice::SerialDevice(boost::asio::io_service *io, std::string port_name, std::string descriptive_name, int baud_rate)
        {
            // Store Asio io service
            this->io = io;

            this->device_address = port_name;
            this->device_name = descriptive_name;
            this->device_type = DEVICE_SERIAL;
            this->baud_rate = baud_rate;

            // Create new SerialPort object
            this->serial_port = new boost::asio::serial_port(*(this->io));
            // Note: the above does not yet have a destructor utilized
        }

        /**
         * Connect to the device
         */
        void SerialDevice::connect()
        {
            bool connected = false;
            while (!connected)
            {
                try
                {
                    this->serial_port->open(this->device_address);
                }
                catch (const boost::system::system_error &e)
                {
                    spdlog::error("Got error when trying to connect to device at {}: {}", this->device_address, e.code().value());
                }

                spdlog::info("Is Connected ({}): {}", this->device_address, this->serial_port->is_open());
                // Store the status of the socket
                connected = this->serial_port->is_open();

                // Sleep if it didn't connect
                if (!connected)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
            this->serial_port->set_option(boost::asio::serial_port_base::baud_rate(this->baud_rate));
            // Start reading on the socket
            this->run_async_read_until();
        }

        /**
        * Store the callback to be used later
        */
        void SerialDevice::register_callback(device_callback _callback)
        {
            this->callback = _callback;
        }

        /**
        * Write data to the serial port
        */
        int SerialDevice::write_data(std::string data)
        {

            // Add the delimiter
            data.append("\n");

            spdlog::debug("Sending {} chars", data.length());

            // Convert it to a c string
            const char *data_c_str = data.c_str();

            spdlog::info("Sending event: {}", data);

            // Send the data over the serial_port object (which we created earlier)
            size_t ret = boost::asio::write(*(this->serial_port), boost::asio::buffer(data_c_str, data.size()));
            spdlog::debug("Bytes written: {}", ret);

            return data.length();
        }

        void SerialDevice::read_handler(const boost::system::error_code &error, std::size_t bytes_transferred)
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

            if (this->serial_port->is_open())
            {
                // Read it again
                this->run_async_read_until();
            }
        }

        void SerialDevice::run_async_read_until()
        {
            boost::asio::async_read_until(
                *(this->serial_port),
                streambuf,
                '\n',
                boost::bind(
                    &SerialDevice::read_handler,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
        }

        void SerialDevice::close()
        {
            this->serial_port->close();
        }

    } // namespace communication_manager
} // namespace simlink