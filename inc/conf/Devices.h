

#ifndef _DEVICES_H
#define _DEVICES_H

#include "json.h"

#include <string>

namespace simlink
{
	namespace configuration_manager
	{
		void load_devices();

		void add_device(std::string type, std::string address, std::string name);
	} // namespace configuration_manager
} // namespace simlink

#endif