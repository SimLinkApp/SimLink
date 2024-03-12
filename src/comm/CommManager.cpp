

#include "spdlog/spdlog.h"

#include "comm/CommManager.h"

#include "comm/SerialDevice.h"
#include "comm/NetworkDevice.h"

#include "EventMapper.h"
#include "util.h"

#include <vector>

namespace simlink
{
	namespace communication_manager
	{

		std::vector<Device *> device_listing;

		std::unordered_map<std::string, std::pair<std::string, std::string>> event_mapping;
		boost::asio::io_context io;
		std::thread *recv_thread;

		void new_device(COMM_DEVICE_TYPE device_type, std::string device_address, std::string descriptive_name)
		{
			// Create new Device interface object
			Device *device;
			switch (device_type)
			{
			case DEVICE_SERIAL:
				// Create new SerialDevice with baud rate 115200
				device = new SerialDevice(&io, device_address, descriptive_name, 115200);
				break;
			case DEVICE_NETWORK:
				// Create new NetworkDevice
				device = new NetworkDevice(&io, device_address, descriptive_name);
				break;
			default:
				spdlog::error("Failed to create new device: {}: Unknown type ({}, {})", device_type, device_address, descriptive_name);
				return;
			}

			spdlog::info("Registering device of type '{}' with address '{}' and descriptive name '{}'", device_type, device_address, descriptive_name);
			// Register the device's callback
			device->register_callback((device_callback)comm_callback);
			// Add to the listing
			device_listing.push_back(device);
		}

		/**
		 * Callback for devices to call when they have a message. Must be a callback here, and not a return,
		 * because of the buffer-style communication. Multiple commands can be received in one read cycle
		 */
		void comm_callback(std::string cmd)
		{
			spdlog::info("Received command: {}", cmd);
			/*
		// Check if event_mapping contains the command
		if (!map_contains(event_mapping, cmd)) {
			spdlog::error("Received unknown event {}", cmd);
		}

		// Create a pair to hold the value of the event mapping
		std::pair<std::string, std::string> event_value = event_mapping[cmd];
		spdlog::debug("Found pair: ({}, {})", event_value.first, event_value.second);
		spdlog::debug(event_mapping.find(cmd) == event_mapping.end());

		// Send the event on to the mapper. We assume that the parameter will be defined in the commandlisting,
		// and therefore we don't have to parse out a value 
		//event_mapper::write_event(event_value.first, event_value.second);
		*/

			simlink::event_mapper::write_event(cmd, "");
		}

		/**
		 * Start the receive thread
		 */
		void start_recv_thread()
		{
			spdlog::debug("Starting recv thread");
			// Create new thread to run the asio io
			recv_thread = new std::thread(read_loop);
			spdlog::debug("Recv thread started");
		}

		/**
		 * Stop the receive thread
		 */
		void stop_recv_thread()
		{
			spdlog::debug("Stopping recv thread");
			// Stop io
			io.stop();
			spdlog::debug("Stopped asio io");
			// Join thread to wait until it's stopped
			recv_thread->join();
			spdlog::debug("Recv thread stopped");
		}

		/**
		 * Query over all the devices
		 */
		void read_loop()
		{
			for (Device *elem : device_listing)
			{
				spdlog::info("Trying to connect to {}", elem->get_device_name());
				elem->connect();
			}
			spdlog::debug("Running read loop");
			// Read data from io
			io.run();
		}

		/**
		 * Get all devices for use by UI. Returns a shallow copy of the vector
		 */
		std::vector<Device *> get_all_devices()
		{
			// Create a new vector
			std::vector<Device *> outval;
			// Iterate over existing devices
			for (Device *elem : device_listing)
			{
				// Add existing device to the new vector
				outval.push_back(elem);
			}

			// Send it onwards
			return outval;
		}

		/**
		 * Remove a device by its address
		 */
		void remove_device(std::string address)
		{
			// Create a temp int var to hold ID
			int i = 0;

			// Iterate over the devices to find the right one
			for (Device *elem : device_listing)
			{
				if (elem->get_device_address() == address)
				{
					elem->close();
					device_listing.erase(device_listing.begin() + i);
				}
				i += 1;
			}
		}

		void send_all(std::string message)
		{
			for (Device *elem : device_listing)
			{
				elem->write_data(message);
			}
		}
	} // namespace communication_manager
} // namespace simlink