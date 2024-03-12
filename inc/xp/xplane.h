#ifndef _SIMLINK_XPLANE_H
#define _SIMLINK_XPLANE_H

#include <string>

namespace simlink
{
    namespace xp
    {
        void init_xplane(std::string remote_address);
        void register_read(std::string ref);
        void tick();
        void close_xplane();
        void trigger_xplane_event(std::string event_name, std::string parameter);
    } // namespace xp
} // namespace simlink

#endif