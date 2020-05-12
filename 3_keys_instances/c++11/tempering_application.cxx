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

// Tempering application:
// 1) Publishes the temperature
// 2) Subscribes to the lot state
// 3) After "processing" the lot, publishes the lot state

void publish_temperature(
        dds::pub::DataWriter<Temperature>& writer,
        const std::string& sensor_id)
{
    // Create temperature sample for writing
    Temperature temperature;
    while (!shutdown_requested) {
        // Modify the data to be written here
        temperature.sensor_id(sensor_id);
        temperature.degrees(rand() % 3 + 30);  // Random value between 30 and 32

        writer.write(temperature);

        rti::util::sleep(dds::core::Duration::from_millisecs(100));
    }
}

void process_lot(
        dds::sub::DataReader<ChocolateLotState>& lot_state_reader,
        dds::pub::DataWriter<ChocolateLotState>& lot_state_writer)
{
    // Take all samples.  Samples are loaned to application, loan is
    // returned when LoanedSamples destructor called.
    dds::sub::LoanedSamples<ChocolateLotState> samples =
            lot_state_reader.take();

    // Process lots waiting for tempering
    for (const auto& sample : samples) {
        if (sample.info().valid()
            && sample.data().next_station()
                    == StationKind::TEMPERING_CONTROLLER) {
            std::cout << "Processing lot #" << sample.data().lot_id()
                      << std::endl;

            // Send an update that the tempering station is processing lot
            ChocolateLotState updated_state(sample.data());
            updated_state.lot_status(LotStatusKind::PROCESSING);
            updated_state.next_station(StationKind::INVALID_CONTROLLER);
            updated_state.station(StationKind::TEMPERING_CONTROLLER);
            lot_state_writer.write(updated_state);

            // "Processing" the lot.
            rti::util::sleep(dds::core::Duration(5));

            // Exercise #3.1: Since this is the last step in processing,
            // notify the monitoring application that the lot is complete
            // using a dispose

        }
    }
}  // The LoanedSamples destructor returns the loan

void run_example(unsigned int domain_id, const std::string& sensor_id)
{
    // A DomainParticipant allows an application to begin communicating in
    // a DDS domain. Typically there is one DomainParticipant per application.
    // DomainParticipant QoS is configured in USER_QOS_PROFILES.xml
    dds::domain::DomainParticipant participant(domain_id);

    // A Topic has a name and a datatype. Create Topics.
    // Topic names are constants defined in the IDL file.
    dds::topic::Topic<Temperature> temperature_topic(
            participant,
            CHOCOLATE_TEMPERATURE_TOPIC);
    dds::topic::Topic<ChocolateLotState> lot_state_topic(
            participant,
            CHOCOLATE_LOT_STATE_TOPIC);

    // A Publisher allows an application to create one or more DataWriters
    // Publisher QoS is configured in USER_QOS_PROFILES.xml
    dds::pub::Publisher publisher(participant);

    // Create DataWriters of Topics "ChocolateTemperature" & "ChocolateLotState"
    // DataWriter QoS is configured in USER_QOS_PROFILES.xml
    dds::pub::DataWriter<Temperature> temperature_writer(
            publisher,
            temperature_topic);
    dds::pub::DataWriter<ChocolateLotState> lot_state_writer(
            publisher,
            lot_state_topic);

    // A Subscriber allows an application to create one or more DataReaders
    // Subscriber QoS is configured in USER_QOS_PROFILES.xml
    dds::sub::Subscriber subscriber(participant);

    // Create DataReader of Topic "ChocolateLotState".
    // DataReader QoS is configured in USER_QOS_PROFILES.xml
    dds::sub::DataReader<ChocolateLotState> lot_state_reader(
            subscriber,
            lot_state_topic);

    // Obtain the DataReader's Status Condition
    dds::core::cond::StatusCondition status_condition(lot_state_reader);

    // Enable the 'data available' status.
    status_condition.enabled_statuses(
            dds::core::status::StatusMask::data_available());

    // Associate a handler with the status condition. This will run when the
    // condition is triggered, in the context of the dispatch call (see below)
    status_condition.extensions().handler(
            [&lot_state_reader, &lot_state_writer]() {
                    process_lot(lot_state_reader, lot_state_writer);
    });

    // Create a WaitSet and attach the StatusCondition
    dds::core::cond::WaitSet waitset;
    waitset += status_condition;

    // Create a thread to periodically publish the temperature
    std::cout << "ChocolateTemperature Sensor with ID: " << sensor_id
              << " starting" << std::endl;
    std::thread temperature_thread(
            publish_temperature,
            std::ref(temperature_writer),
            std::ref(sensor_id));

    while (!shutdown_requested) {
        // Wait for ChocolateLotState
        std::cout << "waiting for lot" << std::endl;
        waitset.dispatch(dds::core::Duration(10));  // Wait up to 10s for update
    }

    temperature_thread.join();
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
        run_example(arguments.domain_id, arguments.sensor_id);
    } catch (const std::exception& ex) {
        // This will catch DDS exceptions
        std::cerr << "Exception in run_example(): " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    // Releases the memory used by the participant factory.  Optional at
    // application shutdown
    dds::domain::DomainParticipant::finalize_participant_factory();

    return EXIT_SUCCESS;
}
