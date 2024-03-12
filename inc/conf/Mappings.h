

#ifndef _MAPPINGS_H
#define _MAPPINGS_H

#include "json.h"

#include <string>

namespace simlink
{
	namespace configuration_manager
	{
		void load_mappings();
		void save_new_read_mapping(std::string raw_name, std::string sim_name);
		void save_new_write_mapping(std::string raw_name, std::string sim_name, std::string parameter);
		void remove_read_mapping(std::string raw_name);
		void remove_write_mapping(std::string raw_name);
	} // namespace configuration_manager
} // namespace simlink

#endif