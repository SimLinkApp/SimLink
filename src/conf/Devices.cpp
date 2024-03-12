

#include "conf/Devices.h"

#include "json.h"
#include "spdlog/spdlog.h"

#include "comm/CommManager.h"
#include "conf/Configuration.h"

namespace simlink
{
	namespace configuration_manager
	{

		/**
	 * Load the configuration from a single ConfigurationDevice struct
	 */
		void load_single_device(const ConfigurationDevice *device)
		{
			spdlog::debug("Loading single device: {}", device->address);
			// Define a temporary variable to store the device type
			simlink::communication_manager::COMM_DEVICE_TYPE device_type;
			// If it's a serial device, store the serial type
			if (device->type == "serial")
			{
				device_type = simlink::communication_manager::DEVICE_SERIAL;
			}
			// If it's a network device, store the network type
			else if (device->type == "network")
			{
				device_type = simlink::communication_manager::DEVICE_NETWORK;
			}
			// Otherwise, throw an error
			else
			{
				spdlog::error("Tried to load device with unknown type");
				return;
			}

			// Create a new device in the Communication Manager with the parameters defined
			// in the configuration
			new_device(device_type, device->address, device->name);
		}

		/**
		 * Load devices from the configuration into the system
		 */
		void load_devices()
		{
			spdlog::debug("Loading devices configuration");
			// If there are no devices, we can assume the devices entry is missing or
			// unpopulated, therefore, throw an error
			if (current_configuration.devices.empty())
			{
				spdlog::error("Configuration file is missing \"devices\" entry");
				return;
			}

			// Iterate over all the devices in a zero-copy manner. We don't need to
			// modify the memory, we just need to copy some values in it
			for (const auto &d : current_configuration.devices)
			{
				// Load a single device by pointer
				load_single_device(&d);
			}
			spdlog::debug("Finished loading configuration devices");
		}

		/**
		 * Add a device to the configuration
		 */
		void add_device(std::string type, std::string address, std::string name)
		{
			spdlog::debug("Saving new device to conf: {}, {}, {}", type, address, name);
			// Create a device object
			ConfigurationDevice device;
			device.type = type;
			device.address = address;
			device.name = name;
			// Store it in configuration
			current_configuration.devices.push_back(device);

			// Save the file
			save_configuration();
		}
	} // namespace configuration_manager
} // namespace simlink