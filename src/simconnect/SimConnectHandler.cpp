#if defined(_WIN32) || defined(_WIN64)

#include "spdlog/spdlog.h"

#include "simconnect/SimConnectHandler.h"

#include <string>
#include <windows.h>
#include <iostream>
#include <unordered_map>
#include <strsafe.h>

#include "simconnect/SimConnect.h"

#include "simconnect/SimConnectDataReceiver.h"
#include "simconnect/SimConnectNotificationReceiver.h"

namespace simlink
{
    namespace simconnect_handler
    {

        HANDLE hSimConnect = NULL;
        HRESULT hr;

        bool startup_query = true;

        enum GROUP_ID
        {
            GROUP_A,
        };

        std::unordered_map<std::string, int> simconnect_event_map;
        std::unordered_map<int, std::string> inv_simconnect_event_map;

        /**
     * Accept a SimConnect notification ID, and translate that to a string
     */
        void handle_internal_simconnect_notification(int event_id, int data)
        {
            // Check if the event ID is in the mapping
            if (inv_simconnect_event_map.find(event_id) == inv_simconnect_event_map.end())
            {
                spdlog::debug("SimConnect triggered an unknown event notification: {}", event_id);
                return;
            }
            // Get the name of the event from the mapping
            std::string event_name = inv_simconnect_event_map[event_id];

            // Send it offwards towards serial connections
            handle_simconnect_notification("simconnect:" + event_name, std::to_string(data));
        }

        /**
     * Much in the same way I registered a simconnect event, I am now mapping a simconnect event to an int, and registering
     * that event in a notification for me to receive.
     */
        void register_simconnect_notification(std::string event_name)
        {
            spdlog::debug("Trying to map notification ", event_name);

            register_simconnect_event(event_name);

            int event_id = simconnect_event_map[event_name];

            spdlog::debug("Notification event {} with id {} mapped ", event_name, event_id);

            hr = SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_A, event_id, false);
        }

        /**
     * Accept a SimConnect event name, and register it with the SimConnect client
     */
        void register_simconnect_event(std::string event_name)
        {
            spdlog::debug("Trying to map {}", event_name);

            // If the event is already mapped, ignore it
            if (simconnect_event_map.find(event_name) != simconnect_event_map.end())
            {
                spdlog::debug("Trying to map already mapped event {}", event_name);
                return;
            }

            // Create a new event ID
            int event_id = simconnect_event_map.size();
            simconnect_event_map[event_name] = event_id;
            inv_simconnect_event_map[event_id] = event_name;

            spdlog::debug("Mapped {} to {}", event_name, event_id);

            // Map the event. This is where I got clever. Instead of using enums, I just casted an int to the SIMCONNECT_CLIENT_EVENT_ID
            // type, and let it have fun. Surprisingly, this works!
            hr = SimConnect_MapClientEventToSimEvent(hSimConnect, (SIMCONNECT_CLIENT_EVENT_ID)event_id, event_name.c_str());

            spdlog::debug("Event id {} registered in simconnect", event_id);
        }

        /**
     * Map a client data name and data ID. Used by addons
     */
        void simconnect_map_client_data_name(std::string data_name, int data_id)
        {
            spdlog::debug("Mapping client data name: {}, {}", data_name, data_id);

            SIMCONNECT_CLIENT_DATA_ID sc_data_id = (SIMCONNECT_CLIENT_DATA_ID)data_id;
            hr = SimConnect_MapClientDataNameToID(hSimConnect, data_name.c_str(), sc_data_id);
        }

        /**
     * Map a client data definition and data size. Used by addons
     */
        void simconnect_add_to_client_data_definition(int data_definition, int size)
        {
            spdlog::debug("Adding client data definition: {}, {}", data_definition, size);

            hr = SimConnect_AddToClientDataDefinition(hSimConnect, data_definition, 0, size, 0, 0);
        }

        /**
     * Map a client data definition and data size. Used by addons
     */
        void simconnect_add_to_data_definition(int data_definition, std::string data_name, SIMCONNECT_DATATYPE datatype)
        {
            spdlog::debug("Adding to data definition: {}, {}", data_name, data_definition);

            hr = SimConnect_AddToDataDefinition(hSimConnect, data_definition, data_name.c_str(), NULL, datatype);
        }

        void simconnect_request_data_on_simobject(int request_id)
        {
            spdlog::debug("Requested data on simobject: {}", request_id);

            SIMCONNECT_DATA_REQUEST_FLAG req_flags = SIMCONNECT_DATA_REQUEST_FLAG_CHANGED;
            hr = SimConnect_RequestDataOnSimObject(hSimConnect, (DWORD)request_id, (DWORD)request_id,
                                                   SIMCONNECT_OBJECT_ID_USER, SIMCONNECT_PERIOD_SECOND,
                                                   req_flags);
        }

        /**
     * Trigger a SimConnect event by string name, accepting a parameter. The parameter should be 0 if it is not needed
     */
        void trigger_simconnect_event(std::string event_name, int parameter)
        {

            // Get the SimConnect event ID mapping I *cleverly* created earlier
            int event_id = simconnect_event_map[event_name];

            spdlog::info("Triggering event {} with id {} and parameter {}", event_name, event_id, parameter);

            /**
         * Convert the integer ID into the SIMCONNECT_CLIENT_EVENT_ID type
         */
            SIMCONNECT_CLIENT_EVENT_ID sc_event_id = (SIMCONNECT_CLIENT_EVENT_ID)event_id;

            spdlog::debug("Event ID: {}", sc_event_id);

            // Transmit the client event, and off it goes into the neverland that is whatever ESP wants to be
            SimConnect_TransmitClientEvent(hSimConnect, 0, sc_event_id, parameter, SIMCONNECT_GROUP_PRIORITY_HIGHEST, SIMCONNECT_EVENT_FLAG_GROUPID_IS_PRIORITY);
        }

        /**
     * SimConnect's dispatch process. Don't call this, SimConnect calls it whenever it's drunk enough to
     */
        void CALLBACK simconnect_dispatch_proc(SIMCONNECT_RECV *pData, DWORD cbData, void *pContext)
        {

            spdlog::debug("Callback called with ID {}", pData->dwID);

            switch (pData->dwID)
            {
            case SIMCONNECT_RECV_ID_EVENT:
            {
                SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT *)pData;

                int event_id = evt->uEventID;
                int data = evt->dwData;

                spdlog::debug("Received event from SimConnect with event id {}", event_id);

                handle_internal_simconnect_notification(event_id, data);

                break;
            }
            case SIMCONNECT_RECV_ID_SIMOBJECT_DATA:
            {
                spdlog::debug("Received SimObject data");

                SIMCONNECT_RECV_SIMOBJECT_DATA *pObjData = (SIMCONNECT_RECV_SIMOBJECT_DATA *)pData;
                handle_received_simobject_data(pObjData);
                break;
            }
            case SIMCONNECT_RECV_ID_EXCEPTION:
            {
                SIMCONNECT_RECV_EXCEPTION *evt = (SIMCONNECT_RECV_EXCEPTION *)pData;
                spdlog::error("Received SimConnect exception {}", evt->dwException);
                break;
            }

            case SIMCONNECT_RECV_ID_QUIT:
            {
                spdlog::info("SimConnect gone!");
                break;
            }

            default:
                break;
            }
        }

        /**
     * Initialize the SimConnect client
     */
        void init_simconnect_events()
        {
            spdlog::info("Trying to connect to Prepar3D");

            while (!SUCCEEDED(SimConnect_Open(&hSimConnect, "SimLink", NULL, 0, 0, 0)) && startup_query)
            {
                Sleep(1000);
            }
            if (startup_query)
            {
                spdlog::info("Connected to Prepar3D");
                hr = SimConnect_SetNotificationGroupPriority(hSimConnect, GROUP_A, SIMCONNECT_GROUP_PRIORITY_HIGHEST);
            }
            else
            {
                spdlog::debug("SimConnect connection closed during open");
            }
        }

        /**
     * Ready the SimConnect connection
     */
        void ready()
        {
        }

        /**
     * Close the SimConnect client
     */
        void close_simconnect()
        {
            spdlog::info("Closing SimConnect");
            startup_query = false;
            hr = SimConnect_Close(hSimConnect);
            spdlog::info("SimConnect closed");
        }

        void tick()
        {
            SimConnect_CallDispatch(hSimConnect, simconnect_dispatch_proc, NULL);
        }
    } // namespace simconnect_handler
} // namespace simlink
#endif