
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <sago/platform_folders.h>

#include "EventMapper.h"

#include "comm/CommManager.h"
#include "conf/Configuration.h"
#include "iface/LuaInterface.h"

#ifdef __APPLE__
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <thread>
#include <chrono>
#include <iostream>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

bool run_main_loop = true;

void signal_callback_handler(int signum)
{
    switch (signum)
    {
    case SIGINT:
        run_main_loop = false;
        break;
    }
}

int main(int argc, char *argv[])
{
    bool debug = false;
    try
    {
        po::options_description desc("Allowed options");
        desc.add_options()("help,h", "print usage message")("verbose,v", po::bool_switch(&debug), "Enable verbose logs");

        po::variables_map vm;
        store(parse_command_line(argc, argv, desc), vm);

        if (vm.count("help"))
        {
            std::cout << desc << "\n";
            return 0;
        }

        debug = vm["verbose"].as<bool>();
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << "\n";
    }

    signal(SIGINT, signal_callback_handler);

    const std::string file_loc = sago::getDataHome() + "/SimLink/simlink.log";

    // Initialize Spdlog
    auto file_logger = spdlog::basic_logger_mt("simlink_logger", file_loc);
    // Write messages to logger

    spdlog::set_default_logger(file_logger);
    file_logger->flush_on(spdlog::level::info);
    spdlog::set_pattern("[%H:%M:%S %z] [thread %t] [%l] %v");

    spdlog::info("********************************");
    spdlog::info("*** Application is starting! ***");
    spdlog::info("********************************");

    if (debug)
    {
        spdlog::info("Setting log level to debug");
        spdlog::set_level(spdlog::level::debug); // Set global log level to debug
        file_logger->flush_on(spdlog::level::debug);
    }

    // Load the configuration from the file into memory
    simlink::configuration_manager::load_configuration();
    // Make sure that any configuration changes are saved
    simlink::event_mapper::set_save_on_change(true);

    spdlog::debug("Calling lua init...");
    // Initialize Lua
    simlink::iface::init_lua();

    // Start the background thread

    simlink::event_mapper::connect_resources();
    simlink::communication_manager::start_recv_thread();

    while (run_main_loop)
    {
        simlink::event_mapper::tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    simlink::event_mapper::close_resources();

    return 0;
}
