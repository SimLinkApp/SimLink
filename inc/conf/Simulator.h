

#ifndef _SIMULATOR_H
#define _SIMULATOR_H

#include "json.h"

#include <string>

namespace simlink
{
	namespace configuration_manager
	{
		void save_simulator_type(std::string simulator_type);
		void load_simulator();
	} // namespace configuration_manager
} // namespace simlink

#endif