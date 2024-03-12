
#ifndef _COMM_MANAGER_H
#define _COMM_MANAGER_H

#include <string>

#include "boost/asio.hpp"

namespace simlink
{
	namespace communication_manager
	{

		enum COMM_DEVICE_TYPE
		{
			DEVICE_SERIAL,
			DEVICE_NETWORK
		};

		typedef void (*device_callback)(std::string command);

		class Device
		{
		public:
			virtual void register_callback(device_callback callback) = 0;
			virtual int write_data(std::string data) = 0;
			virtual void close() = 0;
			virtual void connect() = 0;

			std::string get_device_type()
			{
				switch (this->device_type)
				{
				case DEVICE_SERIAL:
					return "serial";
					break;
				case DEVICE_NETWORK:
					return "network";
					break;
				}
				return "";
			}

			std::string get_device_address()
			{
				return this->device_address;
			}

			std::string get_device_name()
			{
				return this->device_name;
			}

		protected:
			COMM_DEVICE_TYPE device_type;
			std::string device_address;
			std::string device_name;

		private:
			device_callback callback;
		};

		void new_device(COMM_DEVICE_TYPE device_type, std::string device_address, std::string descriptive_name);
		void comm_callback(std::string cmd);
		void read_loop();
		void start_recv_thread();
		void stop_recv_thread();
		std::vector<Device *> get_all_devices();
		void remove_device(std::string address);
		void send_all(std::string message);

	} // namespace communication_manager
} // namespace simlink
#endif