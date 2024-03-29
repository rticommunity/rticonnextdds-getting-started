/*
 * (c) Copyright, Real-Time Innovations, 2020.  All rights reserved.
 * RTI grants Licensee a license to use, modify, compile, and create derivative
 * works of the software solely for use with RTI Connext DDS. Licensee may
 * redistribute copies of the software provided that all such copies are subject
 * to this license. The software is provided "as is", with no warranty of any
 * type, including any warranty for fitness for any purpose. RTI is under no
 * obligation to maintain or support the software. RTI shall not be liable for
 * any incidental or consequential damages arising out of the use or inability
 * to use the software.
 */

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <iostream>
#include <csignal>
#include <string>

#include <dds/core/ddscore.hpp>


namespace application {

// Catch control-C and tell application to shut down
bool shutdown_requested = false;

inline void stop_handler(int)
{
    shutdown_requested = true;
    std::cout << "preparing to shut down..." << std::endl;
}

inline void setup_signal_handlers()
{
    signal(SIGINT, stop_handler);
    signal(SIGTERM, stop_handler);
}

enum class ParseReturn {
    ok,
    failure,
    exit
};

struct ApplicationArguments {
    ParseReturn parse_result;
    unsigned int domain_id;
    unsigned int sample_count;
    std::string sensor_id;
    rti::config::Verbosity verbosity;
    std::string station_kind;
};

// Parses application arguments for example.
inline ApplicationArguments parse_arguments(int argc, char *argv[])
{
    int arg_processing = 1;
    bool show_usage = false;
    ParseReturn parse_result = ParseReturn::ok;
    unsigned int domain_id = 0;
    unsigned int sample_count = (std::numeric_limits<unsigned int>::max)();
    srand((unsigned int)time(NULL));
    std::string sensor_id = std::to_string(rand() % 50);
    std::string station_kind("COCOA_BUTTER_CONTROLLER");
    rti::config::Verbosity verbosity = rti::config::Verbosity::EXCEPTION;

    while (arg_processing < argc) {
        if ((argc > arg_processing + 1)
                && (strcmp(argv[arg_processing], "-d") == 0
                || strcmp(argv[arg_processing], "--domain") == 0)) {
            domain_id = atoi(argv[arg_processing + 1]);
            arg_processing += 2;
        } else if ((argc > arg_processing + 1)
                && (strcmp(argv[arg_processing], "-s") == 0
                || strcmp(argv[arg_processing], "--sample-count") == 0)) {
            sample_count = atoi(argv[arg_processing + 1]);
            arg_processing += 2;
        } else if ((argc > arg_processing + 1)
                && (strcmp(argv[arg_processing], "-v") == 0
                || strcmp(argv[arg_processing], "--verbosity") == 0)) {
            verbosity =
                    static_cast<rti::config::Verbosity::inner_enum>(
                            atoi(argv[arg_processing + 1]));
            arg_processing += 2;
        } else if ((argc > arg_processing + 1)
                && (strcmp(argv[arg_processing], "-i") == 0
                || strcmp(argv[arg_processing], "--sensor-id") == 0)) {
            sensor_id = argv[arg_processing + 1];
            arg_processing += 2;
        } else if ((argc > arg_processing + 1) 
                && (strcmp(argv[arg_processing], "-k") == 0
                || strcmp(argv[arg_processing], "--station-kind") == 0)) {
            station_kind = argv[arg_processing + 1];
            arg_processing += 2;
        } else if (strcmp(argv[arg_processing], "-h") == 0
                || strcmp(argv[arg_processing], "--help") == 0) {
            std::cout << "Example application." << std::endl;
            show_usage = true;
            parse_result = ParseReturn::exit;
            break;
        } else {
            std::cout << "Bad parameter." << std::endl;
            show_usage = true;
            parse_result = ParseReturn::failure;
            break;
        }
    }
    if (show_usage) {
        std::cout << "Usage:\n"\
                    "    -d, --domain        <int>   Domain ID this application will\n" \
                    "                                subscribe in.  \n"
                    "                                Default: 0\n"\
                    "    -s, --sample-count  <int>   Number of samples to receive before\n"\
                    "                                cleanly shutting down. \n"
                    "                                Default: infinite\n"
                    "                                cleanly shutting down. \n"
                    "    -i, --sensor-id    <string> Unique ID of temperature sensor.\n"\
                    "                                Used only by tempering application.\n"\
                    "    -k, --station-kind <string> The type of ingredient station to start.\n"\
                    "                                Used only by ingredient application.\n"\
                    "                                Values:\n"\
                    "                                   COCOA_BUTTER_CONTROLLER,\n"\
                    "                                   SUGAR_CONTROLLER,\n"\
                    "                                   MILK_CONTROLLER,\n"\
                    "                                   VANILLA_CONTROLLER\n"\
                    "    -v, --verbosity     <int>   How much debugging output to show.\n"\
                    "                                Range: 0-5 \n"
                    "                                Default: 0"
                << std::endl;
    }

    return { parse_result, domain_id, sample_count, sensor_id, verbosity, station_kind };
}

}  // namespace application

#endif  // APPLICATION_HPP
