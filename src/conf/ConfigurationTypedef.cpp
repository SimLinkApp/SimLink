

#include "conf/ConfigurationTypedef.h"

#include "json.h"

namespace simlink
{

	namespace configuration_manager
	{

		// Each of these are used to convert from a native JSON to the primary configuration
		// types. Don't ask me how it works, I just copied + pasted from the GitHub repo.
		void to_json(nlohmann::json &j, const AccountInfo &c)
		{
			j = nlohmann::json{
				{"refresh_token", c.refresh_token}};
		}

		void from_json(const nlohmann::json &j, AccountInfo &c)
		{
			if (j.contains("refresh_token"))
			{
				j.at("refresh_token").get_to(c.refresh_token);
			}
		}

		void to_json(nlohmann::json &j, const ConfigurationDevice &c)
		{
			j = nlohmann::json{
				{"name", c.name},
				{"address", c.address},
				{"type", c.type}};
		}

		void from_json(const nlohmann::json &j, ConfigurationDevice &c)
		{
			if (j.contains("name"))
			{
				j.at("name").get_to(c.name);
			}
			if (j.contains("address"))
			{
				j.at("address").get_to(c.address);
			}
			if (j.contains("type"))
			{
				j.at("type").get_to(c.type);
			}
		}

		void to_json(nlohmann::json &j, const ConfigurationMapping &c)
		{
			j = nlohmann::json{
				{"raw_name", c.raw_name},
				{"sim_name", c.sim_name}};
			if (c.parameter != "")
			{
				j["parameter"] = c.parameter;
			}
		}

		void from_json(const nlohmann::json &j, ConfigurationMapping &c)
		{
			if (j.contains("raw_name"))
			{
				j.at("raw_name").get_to(c.raw_name);
			}
			if (j.contains("sim_name"))
			{
				j.at("sim_name").get_to(c.sim_name);
			}
			if (j.contains("parameter"))
			{
				j.at("parameter").get_to(c.parameter);
			}
		}

		void to_json(nlohmann::json &j, const ConfigurationMappingGroup &c)
		{
			j = nlohmann::json{
				{"read", c.read},
				{"write", c.write}};
		}

		void from_json(const nlohmann::json &j, ConfigurationMappingGroup &c)
		{
			if (j.contains("read"))
			{
				j.at("read").get_to(c.read);
			}
			if (j.contains("write"))
			{
				j.at("write").get_to(c.write);
			}
		}

		void to_json(nlohmann::json &j, const ConfigurationSimulator &c)
		{
			j = nlohmann::json{{"type", c.type}};
		}

		void from_json(const nlohmann::json &j, ConfigurationSimulator &c)
		{
			if (j.contains("type"))
			{
				j.at("type").get_to(c.type);
			}
		}

		void to_json(nlohmann::json &j, const Configuration &c)
		{
			j = nlohmann::json{{"devices", c.devices}, {"mappings", c.mappings}, {"simulator", c.simulator}, {"account", c.account}};
		}

		void from_json(const nlohmann::json &j, Configuration &c)
		{
			if (j.contains("devices"))
			{
				j.at("devices").get_to(c.devices);
			}
			if (j.contains("mappings"))
			{
				j.at("mappings").get_to(c.mappings);
			}
			if (j.contains("simulator"))
			{
				j.at("simulator").get_to(c.simulator);
			}
			if (j.contains("account"))
			{
				j.at("account").get_to(c.account);
			}
		}
	} // namespace configuration_manager
} // namespace simlink