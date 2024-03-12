
#ifndef _LUA_INTERFACE_H
#define _LUA_INTERFACE_H

#include <string>

namespace simlink
{
    namespace iface
    {
        void trigger_write(std::string command, std::string parameter);
        void init_lua();
        bool handle_write(std::string command, std::string parameter);
    } // namespace iface
} // namespace simlink

#endif