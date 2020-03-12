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
#include <thread>

#include <dds/pub/ddspub.hpp>
#include <dds/sub/ddssub.hpp>
#include <rti/util/util.hpp>  // for sleep()
#include <rti/config/Logger.hpp>  // for logging
// Or simply include <dds/dds.hpp> 

#include "chocolate_factory.hpp"
#include "application.hpp"  // Argument parsing

using namespace application;

void publish_start_lot(
        dds::pub::DataWriter<ChocolateLotState>& writer, 
        unsigned int& sample_count) 
{
    // Create data sample for writing
    ChocolateLotState sample;
    for (int count = 0; running && (count < sample_count || sample_count == 0);
         count++) {
        // Modify the data to be written here
        sample.lot_id(count % 100);
        sample.lot_status(LotStatusKind::WAITING);
        sample.next_station(StationKind::TEMPERING_CONTROLLER);

        std::cout << "Start lot with ID " << sample.lot_id()
                  << " and next_station: " << sample.next_station()
                  << std::endl;

        writer.write(sample);

        rti::util::sleep(dds::core::Duration(8));
    }
}

unsigned int monitor_lot_state(dds::sub::DataReader<ChocolateLotState>& reader)
{
    // Take all samples.  Samples are loaned to application, loan is
    // returned when LoanedSamples destructor called.
    unsigned int samples_read = 0;
    dds::sub::LoanedSamples<ChocolateLotState> samples = reader.take();

    // Send an update to station that there are lots waiting for tempering
    for (const auto& sample : samples) {
        if (sample.info().valid()) {
            std::cout << sample.data() << std::endl;
            samples_read++;
        }
        // Exercise #1: Detect that a lot is complete by checking for 
        // the disposed state.
    }

    return samples_read;
}  // The LoanedSamples destructor returns the loan


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
    // "ChocolateLotState" with type ChocolateLotState
    dds::topic::Topic<ChocolateLotState> topic(
            participant,
            "ChocolateLotState");

    // A Publisher allows an application to create one or more DataWriters
    // Publisher QoS is configured in USER_QOS_PROFILES.xml
    dds::pub::Publisher publisher(participant);

    // This DataWriter writes data on Topic "ChocolateLotState"
    // DataWriter QoS is configured in USER_QOS_PROFILES.xml
    dds::pub::DataWriter<ChocolateLotState> writer(publisher, topic);

    // A Subscriber allows an application to create one or more DataReaders
    // Subscriber QoS is configured in USER_QOS_PROFILES.xml
    dds::sub::Subscriber subscriber(participant);

    // Create DataReader of Topic "ChocolateLotState".
    // DataReader QoS is configured in USER_QOS_PROFILES.xml
    dds::sub::DataReader<ChocolateLotState> reader(subscriber, topic);
    // Exercise 2: Add a DataReader for Temperature to this application

    // Obtain the DataReader's Status Condition
    dds::core::cond::StatusCondition status_condition(reader);

    // Enable the 'data available' status.
    status_condition.enabled_statuses(
            dds::core::status::StatusMask::data_available());

    // Associate a handler with the status condition. This will run when the
    // condition is triggered, in the context of the dispatch call (see below)
    unsigned int samples_read = 0;
    status_condition.extensions().handler([&reader, &samples_read]() {
        samples_read += monitor_lot_state(reader);
    });

    // Create a WaitSet and attach the StatusCondition
    dds::core::cond::WaitSet waitset;
    waitset += status_condition;

    // Create a thread to periodically publish the temperature
    std::thread start_lot_thread(
            publish_start_lot,
            std::ref(writer), 
            std::ref(sample_count));

    while (running && (samples_read < sample_count || sample_count == 0)) {
        // Dispatch will call the handlers associated to the WaitSet conditions
        // when they activate
        waitset.dispatch(dds::core::Duration(4));  // Wait up to 4s each time
    }

    start_lot_thread.join();
}

// Sets Connext verbosity to help debugging
void set_verbosity(rti::config::Verbosity verbosity)
{
    rti::config::Logger::instance().verbosity(verbosity);
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

    // Enables different levels of debugging output
    set_verbosity(arguments.verbosity);

    try {
        run_example(
                arguments.domain_id,
                arguments.sample_count,
                arguments.sensor_id);
    } catch (const std::exception& ex) {
        // This will catch DDS exceptions
        std::cerr << "Exception in publisher_main(): " << ex.what()
                  << std::endl;
        return EXIT_FAILURE;
    }

    // Releases the memory used by the participant factory.  Optional at
    // application shutdown
    dds::domain::DomainParticipant::finalize_participant_factory();

    return EXIT_SUCCESS;
}
