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

#include <iostream>

#include <dds/pub/ddspub.hpp>
#include <rti/util/util.hpp>  // for sleep()
#include <rti/config/Logger.hpp>  // for logging
// Or simply include <dds/dds.hpp> 

#include "temperature.hpp"
#include "application.hpp"  // Argument parsing

using namespace application;

void run_example(
        unsigned int domain_id,
        unsigned int sample_count,
        const std::string& sensor_id)
{
    // A DomainParticipant allows an application to begin communicating in
    // a DDS domain. Typically there is one DomainParticipant per application.
    // DomainParticipant QoS is configured in USER_QOS_PROFILES.xml
    dds::domain::DomainParticipant participant(domain_id);

    // A Topic has a name and a datatype. Create a Topic named
    // "ChocolateTemperature" with type Temperature
    dds::topic::Topic<Temperature> topic(participant, "ChocolateTemperature");

    // A Publisher allows an application to create one or more DataWriters
    // Publisher QoS is configured in USER_QOS_PROFILES.xml
    dds::pub::Publisher publisher(participant);

    // This DataWriter writes data on Topic "ChocolateTemperature"
    // DataWriter QoS is configured in USER_QOS_PROFILES.xml
    dds::pub::DataWriter<Temperature> writer(publisher, topic);

    // Create data sample for writing
    Temperature sample;
    for (unsigned int count = 0;
         !shutdown_requested && count < sample_count;
         count++) {
        // Modify the data to be written here
        sample.sensor_id(sensor_id);
        sample.degrees(rand() % 3 + 30);  // Random number between 30 and 32

        std::cout << "Writing ChocolateTemperature, count " << count
                  << std::endl;

        writer.write(sample);

        // Exercise: Change this to sleep 100 ms in between writing temperatures
        rti::util::sleep(dds::core::Duration(4));
    }
}

int main(int argc, char *argv[])
{
    // Parse arguments and handle control-C
    auto arguments = parse_arguments(argc, argv);
    if (arguments.parse_result == ParseReturn::PARSE_RETURN_EXIT) {
        return EXIT_SUCCESS;
    } else if (arguments.parse_result == ParseReturn::PARSE_RETURN_FAILURE) {
        return EXIT_FAILURE;
    }
    setup_signal_handlers();

    // Sets Connext verbosity to help debugging
    rti::config::Logger::instance().verbosity(arguments.verbosity);

    try {
        run_example(
                arguments.domain_id,
                arguments.sample_count,
                arguments.sensor_id);
    } catch (const std::exception& ex) {
        // This will catch DDS exceptions
        std::cerr << "Exception in run_example(): " << ex.what()
                  << std::endl;
        return EXIT_FAILURE;
    }

    // Releases the memory used by the participant factory.  Optional at
    // application shutdown
    dds::domain::DomainParticipant::finalize_participant_factory();

    return EXIT_SUCCESS;
}
