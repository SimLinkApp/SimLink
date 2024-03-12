

#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include "conf/ConfigurationTypedef.h"

namespace simlink
{
	namespace configuration_manager
	{

		extern Configuration current_configuration;

		std::string read_file();
		void save_to_file(std::string data);
		void save_configuration();
		void load_configuration();

	} // namespace configuration_manager
} // namespace simlink

#endif