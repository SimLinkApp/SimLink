#ifndef SIMCONNECTHANDLER_H
#define SIMCONNECTHANDLER_H

#if defined(_WIN32) || defined(_WIN64)

#include <string>
#include <windows.h>
#include "simconnect/SimConnect.h"

namespace simlink
{
    namespace simconnect_handler
    {
        void CALLBACK simconnect_dispatch_proc(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext);
        void register_simconnect_notification(std::string event_name);
        void register_simconnect_event(std::string event_name);
        void simconnect_add_to_data_definition(int data_definition, std::string data_name, SIMCONNECT_DATATYPE datatype);
        void simconnect_request_data_on_simobject(int request_id);
        void simconnect_map_client_data_name(std::string data_name, int data_id);
        void simconnect_add_to_client_data_definition(int data_definition, int size);
        void trigger_simconnect_event(std::string event_name, int parameter);
        void init_simconnect_events();
        void close_simconnect();
        void tick();
    } // namespace simconnect_handler
} // namespace simlink

#endif

#endif // SIMCONNECTHANDLER_H
