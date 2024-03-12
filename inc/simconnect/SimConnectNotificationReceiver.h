#ifndef SIMCONNECTEVENTRECEIVER_H
#define SIMCONNECTEVENTRECEIVER_H

#if defined(_WIN32) || defined(_WIN64)

#include <string>

namespace simlink
{
    namespace simconnect_handler
    {
        void handle_simconnect_notification(std::string event_name, std::string event_data);
    }
} // namespace simlink

#endif

#endif // SIMCONNECTEVENTRECEIVER_H
