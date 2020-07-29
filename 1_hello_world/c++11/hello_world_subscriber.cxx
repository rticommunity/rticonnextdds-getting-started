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

#include <algorithm>
#include <iostream>

#include <dds/sub/ddssub.hpp>
#include <dds/core/ddscore.hpp>
#include <rti/config/Logger.hpp>  // for logging
// Or simply include <dds/dds.hpp> 

#include "hello_world.hpp"
#include "application.hpp"  // Argument parsing

using namespace application;

unsigned int process_data(dds::sub::DataReader<HelloWorld>& reader)
{
    // Take all samples.  Samples are loaned to application, loan is
    // returned when LoanedSamples destructor called.
    unsigned int samples_read = 0;
    dds::sub::LoanedSamples<HelloWorld> samples = reader.take();
    for (const auto& sample : samples) {
        if (sample.info().valid()) {
            samples_read++;
            std::cout << sample.data() << std::endl;
        }
    }

    return samples_read;
}

void run_example(unsigned int domain_id, unsigned int sample_count)
{
    // A DomainParticipant allows an application to begin communicating in
    // a DDS domain. Typically there is one DomainParticipant per application.
    // Create a DomainParticipant with default Qos
    dds::domain::DomainParticipant participant(domain_id);

    // A Topic has a name and a datatype. Create a Topic named
    // "HelloWorld Topic" with type HelloWorld
    dds::topic::Topic<HelloWorld> topic(participant, "Example HelloWorld");

    // A Subscriber allows an application to create one or more DataReaders
    // Subscriber QoS is configured in USER_QOS_PROFILES.xml
    dds::sub::Subscriber subscriber(participant);

    // This DataReader will read data of type HelloWorld on Topic
    // "HelloWorld Topic". DataReader QoS is configured in
    // USER_QOS_PROFILES.xml
    dds::sub::DataReader<HelloWorld> reader(subscriber, topic);

    // Obtain the DataReader's Status Condition
    dds::core::cond::StatusCondition status_condition(reader);

    // Enable the 'data available' status.
    status_condition.enabled_statuses(
            dds::core::status::StatusMask::data_available());

    // Associate a handler with the status condition. This will run when the
    // condition is triggered, in the context of the dispatch call (see below)
    unsigned int samples_read = 0;
    status_condition.extensions().handler([&reader, &samples_read]() {
        samples_read += process_data(reader);
    });

    // Create a WaitSet and attach the StatusCondition
    dds::core::cond::WaitSet waitset;
    waitset += status_condition;

    while (!shutdown_requested && samples_read < sample_count) {
        // Dispatch will call the handlers associated to the WaitSet conditions
        // when they activate
        std::cout << "HelloWorld subscriber sleeping for 4 sec..."
                  << std::endl;

        waitset.dispatch(dds::core::Duration(4));  // Wait up to 4s each time
    }
}

int main(int argc, char *argv[])
{
    // Parse arguments and handle control-C
    auto arguments = parse_arguments(argc, argv);
    if (arguments.parse_result == ParseReturn::exit) {
        return EXIT_SUCCESS;
    } else if (arguments.parse_result == ParseReturn::failure) {
        return EXIT_FAILURE;
    }
    setup_signal_handlers();

    // Sets Connext verbosity to help debugging
    rti::config::Logger::instance().verbosity(arguments.verbosity);

    try {
        run_example(arguments.domain_id, arguments.sample_count);
    } catch (const std::exception& ex) {
        // All DDS exceptions inherit from std::exception
        std::cerr << "Exception in run_example(): " << ex.what()
                  << std::endl;
        return EXIT_FAILURE;
    }

    // Releases the memory used by the participant factory.  Optional at
    // application shutdown
    dds::domain::DomainParticipant::finalize_participant_factory();

    return EXIT_SUCCESS;
}
