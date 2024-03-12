
#include "xp/xplane.h"

#include <spdlog/spdlog.h>

#include <queue>
#include <mutex>

#include "xp/xpnetconn.h"

namespace simlink
{
    namespace xp
    {
        std::queue<std::string> queue;
        std::mutex queue_mutex;

        /**
         * Initialize XPlane, connect to the simulator
         */
        void init_xplane(std::string remote_address)
        {
            // Connect to X-Plane
            connect(remote_address);
        }

        /**
         * Notify X-Plane that we want to receive updates
         */
        void register_read(std::string ref)
        {
            spdlog::info("Registering X-Plane read: {}", ref);
            send_message("read:" + ref + "\n");
        }

        void close_xplane()
        {
            disconnect();
        }

        /**
         * Single tick to check if the queue is empty, and if not, send the data onwards
         */
        void tick()
        {
            if (!queue.empty())
            {
                spdlog::debug("Found non-empty X-Plane queue");
                // Lock the mutex
                queue_mutex.lock();
                // Get the first value
                std::string message = queue.front();
                // Remove the first value
                queue.pop();
                // We have the message, so unlock
                queue_mutex.unlock();

                send_message(message);
            }
        }

        /**
         * Send an event to XPlane
         */
        void trigger_xplane_event(std::string event_name, std::string parameter)
        {
            // Create the message to be sent
            std::string message = "write:" + event_name + ":" + parameter + "\n";
            spdlog::info("Triggering X-Plane event: {}", message);
            // Send it to X-Plane

            spdlog::debug("Adding message to X-Plane queue");
            // Lock the mutex
            queue_mutex.lock();
            // Add the message to the queue
            queue.push(message);
            // Unlock the mutex
            queue_mutex.unlock();
        }
    } // namespace xp
} // namespace simlink