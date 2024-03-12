
#include "iface/LuaInterface.h"

#include <spdlog/spdlog.h>

#ifndef NO_LUA

#define SOL_ALL_SAFETIES_ON 1

#include <sol/sol.hpp>
#include <sago/platform_folders.h>
#include <string>
#include <fstream>
#include <boost/filesystem.hpp>

#include "EventMapper.h"

namespace simlink
{
    namespace iface
    {

        std::string sample_lua = "function handle_write(command, parameter)\n  return false\nend";

        sol::state lua;

        /**
         * Initialize Lua scripts, load files, etc.
         */
        void init_lua()
        {
            spdlog::info("Initializing Lua");
            // Open base libraries for Lua
            lua.open_libraries(sol::lib::base);
            spdlog::debug("Initialized Lua libraries");

            // Set trigger_write to a C++ callback
            lua.set_function("trigger_write", trigger_write);
            spdlog::debug("Initialized Lua function registry");
            std::string scripts_dir = sago::getDataHome() + "/SimLink/Scripts/";

            // Check if the scripts directory exists
            if (!boost::filesystem::exists(scripts_dir))
            {
                spdlog::debug("Didn't find existing Lua directory, creating");
                // If not, create it
                boost::filesystem::create_directory(scripts_dir);

                // Create var to hold location of sample lua file
                std::string sample_lua_file = scripts_dir + "sample.lua";
                spdlog::debug("Creating sample Lua file at {}", sample_lua_file);
                // Write sample lua file
                std::ofstream lua_sample_file(sample_lua_file);
                lua_sample_file << sample_lua;
                lua_sample_file.flush();
                lua_sample_file.close();
                spdlog::debug("Created sample Lua file");
            }

            // Iterate over all files in the Scripts directory, and then load them. Allows for global variables to persist w/o complexity
            for (const auto &entry : boost::filesystem::directory_iterator(scripts_dir))
            {
                spdlog::info("Found Lua script: {}", entry.path().string());
                // Load the file
                lua.do_file(entry.path().string());
            }
            spdlog::debug("Finished initializing Lua");
        }

        /**
         * Lua callback to trigger a write event
         */
        void trigger_write(std::string command, std::string parameter)
        {
            spdlog::info("Lua triggered write: {} ({})", command, parameter);
            simlink::event_mapper::sim_write(command, parameter);
        }

        /**
         * Handle write event. Returns `true` if lua handles the event, `false` otherwise
         */
        bool handle_write(std::string command, std::string parameter)
        {
            spdlog::debug("Executing Lua handle_write: {}, {}", command, parameter);
            sol::protected_function f = lua["handle_write"];
            sol::protected_function_result result = f(command, parameter);
            if (result.valid())
            {
                bool resBool = result;
                spdlog::debug("Lua handled event {}: {}", command, resBool);
                return resBool;
            }

            spdlog::error("Lua script returned invalid return type. Must be `boolean`");

            return false;
        }
    } // namespace iface
} // namespace simlink

#else

namespace simlink {
    namespace iface {
        void init_lua() {
            spdlog::info("Lua is disabled");
        }
    }
}

#endif