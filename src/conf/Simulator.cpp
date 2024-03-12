
#include "conf/Simulator.h"

#include "spdlog/spdlog.h"
#include "json.h"

#include "conf/Configuration.h"
#include "EventMapper.h"

namespace simlink
{
	namespace configuration_manager
	{

		/**
		 * Set the simulator type to be stored in the configuration
		 */
		void save_simulator_type(std::string simulator_type)
		{
			spdlog::debug("Saving the simulator type as {} to configuration", simulator_type);

			// Set the type in the configuration object
			current_configuration.simulator.type = simulator_type;

			// Store the value to the file
			save_configuration();
		}

		/**
		 * Load the simulator parameters from the configuration
		 */
		void load_simulator()
		{
			spdlog::debug("Loading simulator configuration");
			// If there's no type, it's either invalid or unpopulated, so throw an error
			if (current_configuration.simulator.type == "")
			{
				spdlog::error("Configuration file is missing \"simulator\" entry");
				return;
			}

			// Get a temp var as the sim type
			std::string sim_type = current_configuration.simulator.type;

			// If the simulator is SimConnect, set the simulator value in the event mapper
			if (sim_type == "simconnect")
			{
				spdlog::debug("Simulator is ESP-based");
				// Set the simulator as SimConnect-based
				simlink::event_mapper::set_simulator(event_mapper::SIMULATOR_TYPE_SIMCONNECT);
			}
			// If the simulator is XPlane, set the simulator value in the event mapper
			else if (sim_type == "xplane")
			{
				spdlog::debug("Simulator is XPlane-based");
				// Set the simulator as XPlane-based
				simlink::event_mapper::set_simulator(event_mapper::SIMULATOR_TYPE_XPLANE);
			}
			// Otherwise, we don't support it, so throw an error
			else
			{
				spdlog::error("Could not determine simulator type");
			}
		}
	} // namespace configuration_manager
} // namespace simlink