
#ifndef _CONFIGURATION_TYPEDEF_H
#define _CONFIGURATION_TYPEDEF_H

#include "json.h"
#include <vector>

namespace simlink
{
	namespace configuration_manager
	{

		struct AccountInfo
		{
			std::string refresh_token;
		};

		struct ConfigurationDevice
		{
			std::string name;
			std::string address;
			std::string type;
		};

		struct ConfigurationMapping
		{
			std::string raw_name;
			std::string sim_name;
			std::string parameter;
			std::string datatype;
		};

		struct ConfigurationMappingGroup
		{
			std::vector<ConfigurationMapping> read;
			std::vector<ConfigurationMapping> write;
		};

		struct ConfigurationSimulator
		{
			std::string type;
		};

		struct Configuration
		{
			AccountInfo account;
			std::vector<ConfigurationDevice> devices;
			ConfigurationMappingGroup mappings;
			ConfigurationSimulator simulator;
		};

		void to_json(nlohmann::json &j, const AccountInfo &c);
		void from_json(const nlohmann::json &j, AccountInfo &c);
		void to_json(nlohmann::json &j, const ConfigurationDevice &c);
		void from_json(const nlohmann::json &j, ConfigurationDevice &c);
		void to_json(nlohmann::json &j, const ConfigurationMapping &c);
		void from_json(const nlohmann::json &j, ConfigurationMapping &c);
		void to_json(nlohmann::json &j, const ConfigurationMappingGroup &c);
		void from_json(const nlohmann::json &j, ConfigurationMappingGroup &c);
		void to_json(nlohmann::json &j, const ConfigurationSimulator &c);
		void from_json(const nlohmann::json &j, ConfigurationSimulator &c);
		void to_json(nlohmann::json &j, const Configuration &c);
		void from_json(const nlohmann::json &j, Configuration &c);

	} // namespace configuration_manager
} // namespace simlink
#endif