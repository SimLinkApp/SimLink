

#include "conf/Configuration.h"

#include "json.h"
#include "spdlog/spdlog.h"
#include <sago/platform_folders.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "EventMapper.h"

#include "conf/Mappings.h"
#include "conf/Devices.h"
#include "conf/Simulator.h"
#include "util.h"

const std::string conf_loc = sago::getDataHome() + "/SimLink/simlink.json";

namespace simlink
{
    namespace configuration_manager
    {

        Configuration current_configuration;

        const std::string default_config = "{\"devices\":[],\"mappings\":{\"read\":[],\"write\":[]},\"simulator\":{}, \"account\": {}}";

        /**
         * Read in the configuration file, and return it as a string
         */
        std::string read_file()
        {
            spdlog::debug("Reading file");

            if (!file_exists(conf_loc))
            {
                spdlog::info("simlink.json file does not exist, creating...");
                save_to_file(default_config);
                spdlog::info("File created");
                spdlog::info("File now extists: {}", file_exists(conf_loc));
            }

            std::ifstream conf_file(conf_loc);
            std::ostringstream data;
            data << conf_file.rdbuf();
            conf_file.close();
            spdlog::debug("File read");

            return data.str();
        }

        /**
         * Save a string to the configuration file
         */
        void save_to_file(std::string data)
        {
            spdlog::debug("Writing file");
            std::ofstream conf_file(conf_loc);
            conf_file << data;
            conf_file.flush();
            conf_file.close();
            spdlog::debug("File written");
        }

        /**
         * Take the current configuration object, and save it to a file
         */
        void save_configuration()
        {
            spdlog::debug("Saving configuration");
            nlohmann::json j = current_configuration;
            save_to_file(j.dump());
            spdlog::debug("Configuration saved");
        }

        /**
         * Take the data read in from the configuration file, and turn it into
         * a usable object.
         */
        void load_configuration()
        {
            spdlog::debug("Loading configuration");
            // Read in the data frrom the file, and parse it to JSON
            nlohmann::json conf = nlohmann::json::parse(read_file());
            spdlog::debug("Configuration loaded");

            // Take that data, and get it as a Configuration struct, using the methods in
            // ConfigurationTypedef.cpp/h
            try
            {
                current_configuration = conf.get<Configuration>();
            }
            catch (nlohmann::json::exception e)
            {
                spdlog::error(e.what());
                return;
            }

            spdlog::debug("Configuration converted to internal object");

            // Load sim configuration
            load_simulator();
            // Load mapping configuration
            load_mappings();
            // Load devices configuration
            load_devices();
        }
    } // namespace configuration_manager
} // namespace simlink