
#if defined(_WIN32) || defined(_WIN64)

#include "simconnect/SimConnectDataReceiver.h"

#include "spdlog/spdlog.h"

#include <string>
#include <windows.h>
#include <unordered_map>
#include <strsafe.h>

#include "simconnect/SimConnect.h"

#include "EventMapper.h"

#include "simconnect/SimConnectHandler.h"

namespace simlink
{
    namespace simconnect_handler
    {

        std::unordered_map<int, std::pair<std::string, SIMCONNECT_DATATYPE>> request_id_mappings;

        /**
     * Handle a received SimObject data value. Converts it to a string, no matter the type, then
     * sends it to a handle read event
     */
        void handle_received_simobject_data(SIMCONNECT_RECV_SIMOBJECT_DATA *data)
        {
            spdlog::debug("Data received");
            // Get the data's request ID
            int request_id = data->dwRequestID;
            // Convert the request ID to a pair of type string & datatype
            std::pair<std::string, SIMCONNECT_DATATYPE> pair = request_id_mappings[request_id];
            // Store the datatype in a local var
            SIMCONNECT_DATATYPE datatype = pair.second;

            spdlog::debug("Data def num: {}", data->dwDefineCount);
            spdlog::debug("Datatype: {}", datatype);
            spdlog::debug("Raw Data: {}", fmt::ptr(&data->dwData));

            // Create an output string which will contain the result
            std::string output_string = "";
            switch (datatype)
            {
            case SIMCONNECT_DATATYPE_INT32:
            {
                // Handle int32 data
                int32_t *data_int = (int32_t *)&data->dwData;
                spdlog::debug("Data (int32): {}", *data_int);
                output_string = std::to_string(*data_int);
                break;
            }
            case SIMCONNECT_DATATYPE_INT64:
            {
                // Handle int64 data
                int64_t *data_int = (int64_t *)&data->dwData;
                spdlog::debug("Data (int64): {}", *data_int);
                output_string = std::to_string(*data_int);
                break;
            }
            case SIMCONNECT_DATATYPE_FLOAT32:
            {
                // Handle float32 data
                float *data_float = (float *)&(data->dwData);
                spdlog::debug("Data (float32): {}", *data_float);
                output_string = std::to_string(*data_float);
                break;
            }
            case SIMCONNECT_DATATYPE_FLOAT64:
            {
                // Handle float64 data
                float *data_float = (float *)&(data->dwData);
                spdlog::debug("Data (float64): {}}", *data_float);
                output_string = std::to_string(*data_float);
                break;
            }
            }

            // Prepend the SimConnect prefix to the output event
            std::string output_name = "simconnect:";
            output_name.append(pair.first);

            // Handle a read event
            event_mapper::handle_read_event(output_name, output_string);
        }

        /**
     * Register a SimConnect data request
     */
        void register_simconnect_datareq(std::string event_name)
        {

            spdlog::debug("Registering data request with event name {}", event_name);

            // Create a new numeric ID for SimConnect
            int event_id = request_id_mappings.size();
            // Set the datatype to float32. Easiest for me
            SIMCONNECT_DATATYPE datatype = SIMCONNECT_DATATYPE_FLOAT64;
            // Store the pair in the mapping
            request_id_mappings[event_id] = std::pair<std::string, SIMCONNECT_DATATYPE>(event_name, datatype);
            // Call SimConnect methods to ensure it knows about my data requests
            simconnect_add_to_data_definition(event_id, event_name, datatype);
            simconnect_request_data_on_simobject(event_id);

            spdlog::debug("Registered {} with id {}", event_name, event_id);
        }
    } // namespace simconnect_handler

} // namespace simlink
#endif