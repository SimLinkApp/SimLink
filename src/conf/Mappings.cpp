

#include "conf/Mappings.h"

#include "spdlog/spdlog.h"
#include "json.h"

#include <string>

#include "EventMapper.h"
#include "conf/Configuration.h"

namespace simlink
{

	namespace configuration_manager
	{

		/**
	 * Load a read event mapping from the configuration
	 */
		void load_read_mapping(const ConfigurationMapping *mapping)
		{
			// If there is no raw name, it's invalid
			if (mapping->raw_name == "")
			{
				spdlog::error("Read configuration entry is missing raw_name");
				return;
			}
			// If there's no sim name, it's invalid
			if (mapping->sim_name == "")
			{
				spdlog::error("Read configuration entry is missing sim_name");
				return;
			}

			// Register the event as a read event in the main mapper system
			spdlog::debug("Registering read mapping from configuration: {} --> {} ({})", mapping->raw_name, mapping->sim_name, mapping->datatype);
			simlink::event_mapper::register_read(mapping->raw_name, mapping->sim_name, mapping->datatype);
		}

		/**
	 * Load a write event mapping from the configuration
	 */
		void load_write_mapping(const ConfigurationMapping *mapping)
		{
			// If there is no raw name, it's invalid
			if (mapping->raw_name == "")
			{
				spdlog::error("Write configuration entry is missing raw_name");
				return;
			}
			// If there's no sim name, it's invalid
			if (mapping->sim_name == "")
			{
				spdlog::error("Write configuration entry is missing sim_name");
				return;
			}

			// Register the event as a write event in the main mapper system
			spdlog::debug("Registering write mapping from configuration: {} --> {}:{}", mapping->raw_name, mapping->sim_name, mapping->parameter);
			simlink::event_mapper::register_write(mapping->raw_name, mapping->sim_name, mapping->parameter);
		}

		/**
	 * Load all mappings from the configuration
	 */
		void load_mappings()
		{
			spdlog::debug("Loading mappings configuration");
			// If both read & write mappings listings are empty, this application is useless, so throw an error
			if (current_configuration.mappings.read.empty() && current_configuration.mappings.write.empty())
			{
				spdlog::error("Configuration file is missing \"mappings\" entry");
				return;
			}

			// Iterate over all read mappings
			spdlog::debug("Loading read mappings");
			for (auto const &m : current_configuration.mappings.read)
			{
				// Load a singular read mapping from the configuration
				load_read_mapping(&m);
			}

			// Iterate over all write mappings
			spdlog::debug("Loading write mappings");
			for (auto const &m : current_configuration.mappings.write)
			{
				// Load a singular write mapping from the configuration
				load_write_mapping(&m);
			}
		}

		/**
	 * Save a read mapping to the configuration, then save the file
	 */
		void save_new_read_mapping(std::string raw_name, std::string sim_name)
		{
			spdlog::debug("Saving a new read mapping: {}, {}", raw_name, sim_name);

			// Check if the mapping already exists
			for (int i = 0; i < current_configuration.mappings.read.size(); i++)
			{
				// Create a temporary variable
				auto m = current_configuration.mappings.read.at(i);

				// If the name matches, return
				if (m.raw_name == raw_name)
				{
					spdlog::debug("Found existing instance of the mapping {}", raw_name);
					return;
				}
			}

			// Create a variable to hold the mapping
			ConfigurationMapping m;
			m.raw_name = raw_name;
			m.sim_name = sim_name;

			spdlog::debug("Pushing the read mapping onto the vector");

			// Push the variable onto the read mappings vector
			current_configuration.mappings.read.push_back(m);

			// Save the configuration file
			save_configuration();

			spdlog::debug("Done saving the new read mapping");
		}

		/**
	 * Save a write mapping to the configuration, then save the file
	 */
		void save_new_write_mapping(std::string raw_name, std::string sim_name, std::string parameter)
		{
			spdlog::debug("Saving a new write mapping: {}, {} ({})", raw_name, sim_name, parameter);

			// Check if the mapping already exists
			for (int i = 0; i < current_configuration.mappings.write.size(); i++)
			{
				// Create a temporary variable
				auto m = current_configuration.mappings.write.at(i);

				// If the name matches, return
				if (m.raw_name == raw_name)
				{
					spdlog::debug("Found existing instance of the mapping {}", raw_name);
					return;
				}
			}

			// Create a variable to hold the mapping
			ConfigurationMapping m;
			m.raw_name = raw_name;
			m.sim_name = sim_name;
			m.parameter = parameter;

			spdlog::debug("Pushing the write mapping onto the vector");

			// Push the variable onto the read mappings vector
			current_configuration.mappings.write.push_back(m);

			// Save the configuration file
			save_configuration();

			spdlog::debug("Done saving the new write mapping");
		}

		/**
	 * Remove a read mapping from the configuration, then save the file
	 */
		void remove_read_mapping(std::string raw_name)
		{
			spdlog::debug("Removing element from read mappings: {}", raw_name);

			// Iterate over all read mappings
			for (int i = 0; i < current_configuration.mappings.read.size(); i++)
			{
				// Create a temporary variable
				auto m = current_configuration.mappings.read.at(i);

				// If the raw name matches
				if (m.raw_name == raw_name)
				{
					spdlog::debug("Found element to remove from read mappings at index {}", i);
					// Remove the element
					current_configuration.mappings.read.erase(current_configuration.mappings.read.begin() + i);

					// Save the configuration file
					save_configuration();
					return;
				}
			}
		}

		/**
	 * Remove a write mapping from the configuration, then save the file
	 */
		void remove_write_mapping(std::string raw_name)
		{
			spdlog::debug("Removing element from reawrited mappings: {}", raw_name);

			// Iterate over all read mappings
			for (int i = 0; i < current_configuration.mappings.write.size(); i++)
			{
				// Create a temporary variable
				auto m = current_configuration.mappings.write.at(i);

				// If the raw name matches
				if (m.raw_name == raw_name)
				{
					spdlog::debug("Found element to remove from write mappings at index {}", i);
					// Remove the element
					current_configuration.mappings.write.erase(current_configuration.mappings.write.begin() + i);

					// Save the configuration file
					save_configuration();
					return;
				}
			}
		}
	} // namespace configuration_manager
} // namespace simlink