#ifndef EVENTMAPPER_H
#define EVENTMAPPER_H

#include <string>
#include <vector>
#include <unordered_map>

namespace simlink
{
    namespace event_mapper
    {
        enum SIMULATOR_TYPE
        {
            SIMULATOR_TYPE_SIMCONNECT,
            SIMULATOR_TYPE_XPLANE
        };

        void register_write(std::string event_name, std::string sim_event_name, std::string parameter);
        void register_read(std::string event_name, std::string sim_event_name, std::string datatype);
        void deregister_write(std::string event_name);
        void deregister_read(std::string event_name);

        void internal_register_write(std::string event_name, std::string sim_event_name, std::string parameter);
        void internal_register_read(std::string event_name, std::string sim_event_name, std::string datatype);

        std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> get_all_write_mappings();
        std::unordered_map<std::string, std::string> get_all_read_mappings();

        void write_event(std::string event_name, std::string event_value);
        void handle_read_event(std::string event_name, std::string event_value);

        void sim_write(std::string name, std::string parameter);

        std::string get_simulator();
        void set_simulator(SIMULATOR_TYPE sim_type);

        void tick();

        void set_save_on_change(bool save_on_change);
        void register_all_events();

        void connect_resources();
        void close_resources();
    } // namespace event_mapper
} // namespace simlink
#endif // EVENTMAPPER_H
