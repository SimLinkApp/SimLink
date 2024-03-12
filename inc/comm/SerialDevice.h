

#ifndef _SERIAL_DEVICE_H
#define _SERIAL_DEVICE_H

#define port_prefix "\\\\.\\"

#include <string>

#include "comm/CommManager.h"

#include "boost/asio.hpp"

namespace simlink
{
	namespace communication_manager
	{

		class SerialDevice : public Device
		{
		public:
			SerialDevice(boost::asio::io_service *io, std::string port_name, std::string descriptive_name, int baud_rate);
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
			boost::asio::serial_port *serial_port;
			int baud_rate;
			SerialDevice();
		};
	} // namespace communication_manager
} // namespace simlink
#endif