
#ifndef _SIMCONNECT_DATA_RECEIVER_H
#define _SIMCONNECT_DATA_RECEIVER_H

#if defined(_WIN32) || defined(_WIN64)

#include <string>
#include <windows.h>
#include "simconnect/SimConnect.h"

namespace simlink
{
    namespace simconnect_handler
    {

        void handle_received_simobject_data(SIMCONNECT_RECV_SIMOBJECT_DATA *data);

        void register_simconnect_datareq(std::string event_name);
    } // namespace simconnect_handler
} // namespace simlink

#endif

#endif