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

#include "hello_world.hpp"
#include "application.hpp"  // Argument parsing

void run_example(int domain_id, int sample_count)
{
    // A DomainParticipant allows an application to begin communicating in
    // a DDS domain. Typically there is one DomainParticipant per application.
    // DomainParticipant QoS is configured in USER_QOS_PROFILES.xml
    dds::domain::DomainParticipant participant(domain_id);

    // A Topic has a name and a datatype. Create a Topic named
    // "Example HelloMessage" with type HelloMessage
    dds::topic::Topic<HelloMessage> topic(participant, "Example HelloMessage");

    // A Publisher allows an application to create one or more DataWriters
    // Publisher QoS is configured in USER_QOS_PROFILES.xml
    dds::pub::Publisher publisher(participant);

    // This DataWriter writes data on Topic "Example HelloMessage"
    // DataWriter QoS is configured in USER_QOS_PROFILES.xml
    dds::pub::DataWriter<HelloMessage> writer(publisher, topic);

    // Create data sample for writing
    HelloMessage sample;
    for (int count = 0; running && (count < sample_count || sample_count == 0);
         count++) {
        // Modify the data to be written here

        std::cout << "Writing HelloMessage, count " << count << std::endl;

        writer.write(sample);

        rti::util::sleep(dds::core::Duration(4));
    }
}

// Sets Connext verbosity to help debugging
void set_verbosity(unsigned int verbosity)
{
    rti::config::Logger::instance().verbosity(
            static_cast<rti::config::Verbosity::inner_enum>(verbosity));
}

int main(int argc, char *argv[])
{
    // Parse arguments and handle control-C
    unsigned int domain_id = 0;
    unsigned int sample_count = 0;  // infinite loop
    unsigned int verbosity = 0;
    ParseReturn parse_return_value =
            parse_arguments(domain_id, sample_count, verbosity, argc, argv);

    if (parse_return_value == EXIT) {
        return EXIT_SUCCESS;
    } else if (parse_return_value == ERROR) {
        return EXIT_FAILURE;
    }

    setup_signal_handlers();

    // Enables different levels of debugging output
    set_verbosity(verbosity);

    try {
        run_example(domain_id, sample_count);
    } catch (const std::exception& ex) {
        // This will catch DDS exceptions
        std::cerr << "Exception in publisher_main(): " << ex.what()
                  << std::endl;
        return EXIT_FAILURE;
    }

    // Releases the memory used by the participant factory.
    dds::domain::DomainParticipant::finalize_participant_factory();

    return EXIT_SUCCESS;
}
