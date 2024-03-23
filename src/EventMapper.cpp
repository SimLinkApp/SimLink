
#include "EventMapper.h"

#include "spdlog/spdlog.h"

#include <unordered_map>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include "simconnect/SimConnectHandler.h"
#include "simconnect/SimConnectDataReceiver.h"
#endif

#include "xp/xplane.h"

#include "conf/Mappings.h"
#include "conf/Simulator.h"
#include "comm/CommManager.h"
#include "iface/LuaInterface.h"
#include "SimLink.h"
#include "util.h"

namespace simlink
{
    namespace event_mapper
    {

        std::unordered_map<std::string, std::vector<WriteEvent>> write_events;
        std::unordered_map<std::string, ReadEvent> read_events;

        SIMULATOR_TYPE simulator_type;

        bool save_on_change = false;
        bool resources_connected = false;

        /**
         * Register a write event. When a write event is received with event_name, convert it
         * into a write event as sim_event_name
         */
        void register_write(std::string event_name, std::string sim_event_name, std::string parameter)
        {
            spdlog::debug("Registering write: {}, {}, {}", event_name, sim_event_name, parameter);
            // Check if the event is already registered. We don't allow duplicates (yet)
            if (write_events.find(event_name) != write_events.end())
            {
                spdlog::error("Tried to register already-registered write event");
                return;
            }

            WriteEvent evt;
            evt.event_name = event_name;
            evt.sim_event_name = sim_event_name;
            evt.parameter = parameter;

            // Add it to the write events listing
            write_events[event_name].push_back(evt);

            if (resources_connected)
            {
                internal_register_write(evt);
            }

            // If it should save changes to the configuration
            if (save_on_change)
            {
                // Save the new mapping to the file
                simlink::configuration_manager::save_new_write_mapping(event_name, sim_event_name, parameter);
            }
        }

        /**
         * Register a read event. When a write event is received with event_name, convert it
         * into a read event as sim_event_name
         */
        void register_read(std::string event_name, std::string sim_event_name, std::string datatype)
        {
            spdlog::debug("Registering read: {}, {} ({})", event_name, sim_event_name, datatype);
            // Check if the event is already registered. We don't allow duplicates (yet)
            if (read_events.find(event_name) != read_events.end())
            {
                spdlog::error("Tried to register already-registered read event");
                return;
            }

            ReadEvent evt;
            evt.event_name = event_name;
            evt.sim_event_name = sim_event_name;
            evt.datatype = datatype;

            // Add it to the read events listing
            read_events[sim_event_name] = evt;

            if (resources_connected)
            {
                internal_register_read(evt);
            }

            // If it should save changes to the configuration
            if (save_on_change)
            {
                // Save the new mapping to the file
                simlink::configuration_manager::save_new_read_mapping(event_name, sim_event_name);
            }
        }

        /**
         * Write an event to the simulator. Accepts a name & a value, then converts it to the sim
         * event style, then sends it via the appropriate method
         */
        void write_event(std::string event_name, std::string event_value)
        {
            // Check if the event exists. If not, error & return
            if (write_events.find(event_name) == write_events.end())
            {
                spdlog::error("Tried to write nonexistent event \({}\)", event_name);
                return;
            }

            spdlog::debug("Asked to write event for {}", event_name);

            spdlog::debug("Checking if Lua handles event");

            // Get the sim's list of event names from the event listing
            std::vector<WriteEvent> sim_events = write_events[event_name];

            // Iterate over all events received from the vector
            for (const auto elem : sim_events)
            {
                spdlog::debug("Writing event {} --> {}:{}", event_name, elem.sim_event_name, elem.parameter);

                // If the event did not have an explicit value associated (wasn't overriden by Lua)
                if (event_value == "")
                {
                    // Then set it to the default value specified in the registered event
                    event_value = elem.parameter;
                }
                sim_write(elem.event_name, event_value);
            }
        }

        /**
         * Skip the translation, *directly* write an event to the simulator
         */
        void sim_write(std::string name, std::string parameter)
        {

            // Switch for the event values
            switch (simulator_type)
            {
            case SIMULATOR_TYPE_SIMCONNECT:
#if defined(_WIN32) || defined(_WIN64)
                // If it still didn't have an event value
                if (parameter == "")
                {
                    // Default to 0 for simconnect
                    parameter = "0";
                }
                // Trigger the appropriate SimConnect event
                simlink::simconnect_handler::trigger_simconnect_event(name, std::stoi(parameter));
#endif
                break;
            case SIMULATOR_TYPE_XPLANE:
                simlink::xp::trigger_xplane_event(name, parameter);
            default:
                break;
            }
        }

        /**
         * Read an event from the simulator. Accepts a name & a value, then converts it to the sim
         * event style, then sends it via the appropriate method
         */
        void handle_read_event(std::string event_name, std::string event_value)
        {
            // Check if the event exists. If not, error & return
            if (read_events.find(event_name) == read_events.end())
            {
                spdlog::error("Tried to read nonexistent event \({}\)", event_name);
                return;
            }

            // Get the device event's name from the event listing
            std::string device_event_name = read_events[event_name].event_name;

            spdlog::debug("Got read event to trigger {} with value {}", device_event_name, event_value);

            simlink::communication_manager::send_all(device_event_name + ":" + event_value);
        }

        /**
         * Remove a write event from the listing
         */
        void deregister_write(std::string event_name)
        {
            spdlog::debug("Removing write event {}", event_name);
            // Check if the event exists. If not, error & return
            if (write_events.find(event_name) == write_events.end())
            {
                spdlog::error("Tried to delete nonexistant write event");
                return;
            }

            // If it should save changes to the configuration
            if (save_on_change)
            {
                // Save the mapping (or lack thereof) to the file
                simlink::configuration_manager::remove_write_mapping(event_name);
            }

            // Remove the event at the given name
            write_events.erase(event_name);
            spdlog::debug("Removed write event {}", event_name);
        }

        /**
         * Remove a read event from the listing
         */
        void deregister_read(std::string event_name)
        {
            spdlog::debug("Removing read event {}", event_name);
            // Check if the event exists. If not, error & return
            if (read_events.find(event_name) == read_events.end())
            {
                spdlog::error("Tried to delete nonexistant read event");
                return;
            }

            // If it should save changes to the configuration
            if (save_on_change)
            {
                // Save the mapping (or lack thereof) to the file
                simlink::configuration_manager::remove_read_mapping(event_name);
            }

            // Remove the event at the given name
            read_events.erase(event_name);
            spdlog::debug("Removed read event {}", event_name);
        }

        /**
         * Internally register a write. DO NOT CALL THIS UNLESS YOU ARE SURE OF WHAT YOU'RE DOING
         */
        void internal_register_write(WriteEvent evt)
        {
            switch (simulator_type)
            {
            case SIMULATOR_TYPE_SIMCONNECT:
#if defined(_WIN32) || defined(_WIN64)
                simlink::simconnect_handler::register_simconnect_event(evt.sim_event_name);
#endif
                break;
            default:
                break;
            }
        }

        /**
         * Internally register a read. DO NOT CALL THIS UNLESS YOU ARE SURE OF WHAT YOU'RE DOING
         */
        void internal_register_read(ReadEvent evt)
        {
            switch (simulator_type)
            {
            case SIMULATOR_TYPE_SIMCONNECT:
#if defined(_WIN32) || defined(_WIN64)
                simlink::simconnect_handler::register_simconnect_datareq(evt.sim_event_name, evt.datatype);
#endif
                break;
            case SIMULATOR_TYPE_XPLANE:
                simlink::xp::register_read(evt.sim_event_name);
                break;
            default:
                break;
            }
        }

        /**
         * Registers all events with the simulator internally.
         */
        void register_all_events()
        {
            spdlog::debug("Registering all events");
            // Iterate over the entire map
            for (std::pair<std::string, std::vector<WriteEvent>> element : write_events)
            {
                // Iterate over all of the mappings in the vector at that position
                for (const auto elem : element.second)
                {
                    internal_register_write(elem);
                }
            }
            spdlog::debug("Registering all write events completed");

            for (std::pair<std::string, ReadEvent> element : read_events)
            {
                // Inverted because we go from sim name to device name
                internal_register_read(element.second);
            }
            spdlog::debug("Registering all read events completed");
        }

        /**
         * Get all write mappings
         */
        std::unordered_map<std::string, std::vector<WriteEvent>> get_all_write_mappings()
        {
            spdlog::debug("All write mappings requested");
            // Create return value
            std::unordered_map<std::string, std::vector<WriteEvent>> retvals;
            // Iterate over all events
            for (std::pair<std::string, std::vector<WriteEvent>> element : write_events)
            {
                // Store in the new return value
                retvals[element.first] = element.second;
                spdlog::debug("Stored {}", element.first);
            }
            return retvals;
        }

        /**
         * Get all read mappings
         */
        std::unordered_map<std::string, ReadEvent> get_all_read_mappings()
        {
            spdlog::debug("All read mappings requested");
            // Create return value
            std::unordered_map<std::string, ReadEvent> retvals;
            // Iterate over all events
            for (std::pair<std::string, ReadEvent> element : read_events)
            {
                // Store in the new return value
                retvals[element.first] = element.second;
            }
            return retvals;
        }

        /**
         * Get the simulator type
         */
        std::string get_simulator()
        {
            spdlog::debug("Getting simulator type");

            switch (simulator_type)
            {
            case SIMULATOR_TYPE_SIMCONNECT:
                return "simconnect";
            case SIMULATOR_TYPE_XPLANE:
                return "xplane";
            default:
                return "";
            }
        }

        /**
         * Sets the currently running simulator type
         */
        void set_simulator(SIMULATOR_TYPE sim_type)
        {
            spdlog::debug("Setting simulator type to {}", sim_type);

            // Switch on simulator type
            switch (sim_type)
            {
            case SIMULATOR_TYPE_SIMCONNECT:
                spdlog::debug("SimConnect sim set");
#if defined(_WIN32) || defined(_WIN64)
                simulator_type = SIMULATOR_TYPE_SIMCONNECT;
                // If we're supposed to save to config on change
                if (save_on_change)
                {
                    // Save simconnect to configuration
                    simlink::configuration_manager::save_simulator_type("simconnect");
                }
#endif
                break;
            case SIMULATOR_TYPE_XPLANE:
                spdlog::debug("X-Plane sim set");
                simulator_type = SIMULATOR_TYPE_XPLANE;
                // If we're supposed to save to config on change
                if (save_on_change)
                {
                    // Save xplane to configuration
                    simlink::configuration_manager::save_simulator_type("xplane");
                }

                break;
            }
        } // namespace event_mapper

        /**
         * One "tick" of the events connection. Trigger queries that need to run, etc.
         */
        void tick()
        {
            switch (simulator_type)
            {
            case SIMULATOR_TYPE_SIMCONNECT:
#if defined(_WIN32) || defined(_WIN64)
                simlink::simconnect_handler::tick();
#endif
                break;
            default:
                simlink::xp::tick();
                break;
            }
        }

        /**
         * Set whether or not SimLink should save changes to the mappings
         */
        void set_save_on_change(bool _save_on_change)
        {
            spdlog::debug("Setting save_on_change to {}", _save_on_change);
            // Store the main variable
            save_on_change = _save_on_change;
        }

        /**
         * Connect any needed resources to start the sim. Should be done from a separate thread
         */
        void connect_resources()
        {
            spdlog::debug("Connecting resources");

            switch (simulator_type)
            {
            case SIMULATOR_TYPE_SIMCONNECT:
#if defined(_WIN32) || defined(_WIN64)
                simlink::simconnect_handler::init_simconnect_events();
#endif
                break;
            case SIMULATOR_TYPE_XPLANE:
                spdlog::debug("Connecting to X-Plane resources");
                // Hard-code for now, will support networked sims in the future
                simlink::xp::init_xplane("127.0.0.1");
                break;
            }
            resources_connected = true;
            spdlog::debug("Resources connected, registering all events");
            register_all_events();
            spdlog::debug("Internal events registered");
        }

        /**
         * Close down any resources used by the EventMapper
         */
        void close_resources()
        {
            spdlog::debug("Closing EventMapper resources");
            switch (simulator_type)
            {
            case SIMULATOR_TYPE_SIMCONNECT:
#if defined(_WIN32) || defined(_WIN64)
                simlink::simconnect_handler::close_simconnect();
#endif
                break;
            default:
                simlink::xp::close_xplane();
                break;
            }
        }

    } // namespace event_mapper
} // namespace simlink