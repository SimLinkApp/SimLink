
#include "xp/xpnetconn.h"

#include <spdlog/spdlog.h>

#include <string>
#include <thread>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/algorithm/string.hpp>

#include "EventMapper.h"

namespace simlink
{
    namespace xp
    {
        std::string xp_sim_addr;

        boost::asio::io_service io_service;
        boost::asio::ip::tcp::socket *socket;
        boost::asio::streambuf streambuf;
        std::thread xp_thread;

        void xp_thread_loop()
        {
            spdlog::info("Started XP thread loop");
            io_service.run();
        }

        /**
         * Connect to the X-Plane simulator over TCP
         */
        void connect(std::string host)
        {
            spdlog::info("Connecting to X-Plane at {}", host);
            // Create the socket object
            socket = new boost::asio::ip::tcp::socket(io_service);
            spdlog::debug("Socket created");
            bool connected = false;
            while (!connected)
            {
                try
                {
                    // Connect to the plugin
                    socket->connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), 5465));
                }
                catch (const boost::system::system_error &e)
                {
                    spdlog::error("Got error when trying to connect to X-Plane: {}", e.code().value());
                }
                spdlog::info("Is Connected to X-Plane: {}", socket->is_open());
                connected = socket->is_open();
                if (!connected)
                {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
            spdlog::info("Connected to X-Plane");
            run_async_read_until();
            xp_thread = std::thread(xp_thread_loop);
        }

        /**
         * Disconnect the socket
         */
        void disconnect()
        {
            spdlog::debug("Closing X-Plane socket");
            socket->close();
            spdlog::debug("X-Plane socket closed");
        }

        /**
         * Handle a read event from the socket
         */
        void handle_read_event(std::string message)
        {
            spdlog::info("Got message from X-Plane socket: {}", message);

            std::vector<std::string> split;
            boost::algorithm::split(split, message, boost::is_any_of(":"));

            simlink::event_mapper::handle_read_event(split[0] + ":" + split[1], split[2]);
        }

        /**
         * Socket read handler
         */
        void read_handler(const boost::system::error_code &error, std::size_t bytes_transferred)
        {
            spdlog::debug("XP read {} bytes", bytes_transferred);

            // Prepare bytes to be transferred
            streambuf.prepare(bytes_transferred);

            // Use C++ magic to read from the port
            std::string cmd{
                buffers_begin(streambuf.data()),
                buffers_begin(streambuf.data()) + bytes_transferred - 1};

            // Run the callback
            handle_read_event(cmd);

            // Consume the bytes
            streambuf.consume(bytes_transferred);

            if (socket->is_open())
            {
                // Read it again
                run_async_read_until();
            }
        }

        /**
         * A kinda callback sorta thing that reads
         */
        void run_async_read_until()
        {
            boost::asio::async_read_until(
                *(socket),
                streambuf,
                '\n',
                &read_handler);
        }

        /**
         * Send a message over TCP to the simulator
         */
        void send_message(std::string message)
        {
            spdlog::debug("Sending message to X-Plane: {}", message);
            // Create temp var to hold error code
            boost::system::error_code error;
            // Write the data
            boost::asio::write(*socket, boost::asio::buffer(message), error);

            // Check for an error
            if (!error)
            {
                spdlog::debug("Sent message of length {} to XPlane", message.size());
            }
            else
            {
                spdlog::error("Error sending message to XPlane: {}", error.message());
            }
        }
    } // namespace xp
} // namespace simlink