
#if defined(_WIN32) || defined(_WIN64)

#include "simconnect/SimConnectNotificationReceiver.h"

#include "spdlog/spdlog.h"

namespace simlink
{
    namespace simconnect_handler
    {

        void handle_simconnect_notification(std::string event_name, std::string event_data)
        {
            spdlog::debug("Received simconnect notification {} with data {}", event_name, event_data);
        }
    } // namespace simconnect_handler
} // namespace simlink
#endif