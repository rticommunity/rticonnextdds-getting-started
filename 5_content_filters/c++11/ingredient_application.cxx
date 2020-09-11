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

// Ingredient application:
// 1) Subscribes to the lot state
// 2) "Processes" the lot. (In this example, that means sleep for a time)
// 3) After "processing" the lot, publishes an updated lot state

void process_lot(
        StationKind station_kind,
        const std::map<StationKind, StationKind>& next_station,
        dds::sub::DataReader<ChocolateLotState>& lot_state_reader,
        dds::pub::DataWriter<ChocolateLotState>& lot_state_writer)
{
    // Take all samples.  Samples are loaned to application, loan is
    // returned when LoanedSamples destructor called.
    dds::sub::LoanedSamples<ChocolateLotState> samples =
            lot_state_reader.take();

    // Process lots waiting for tempering
    for (const auto& sample : samples) {
        // Exercise #1.3: Remove the check that the Tempering Application is
        // the next_station. This will now be filtered automatically.
        if (sample.info().valid() && !shutdown_requested) {
            std::cout << "Processing lot #" << sample.data().lot_id()
                      << std::endl;

            // Send an update that this station is processing lot
            ChocolateLotState updated_state(sample.data());
            updated_state.lot_status(LotStatusKind::PROCESSING);
            updated_state.next_station(StationKind::INVALID_CONTROLLER);
            updated_state.station(station_kind);
            lot_state_writer.write(updated_state);

            // "Processing" the lot.
            rti::util::sleep(dds::core::Duration(5));

            // Send an update that this station is done processing lot
            updated_state.lot_status(LotStatusKind::COMPLETED);
            updated_state.next_station(next_station[station_kind]);
            updated_state.station(station_kind);
            lot_state_writer.write(updated_state);
        }
    }
}  // The LoanedSamples destructor returns the loan

StationKind string_to_stationkind(const std::string& station_kind)
{
    if (station_kind == "SUGAR_CONTROLLER") {
        return StationKind_def::SUGAR_CONTROLLER;
    } else if (station_kind == "COCOA_BUTTER_CONTROLLER") {
        return StationKind_def::COCOA_BUTTER_CONTROLLER;
    } else if (station_kind == "MILK_CONTROLLER") {
        return StationKind_def::MILK_CONTROLLER;
    } else if (station_kind == "VANILLA_CONTROLLER") {
        return StationKind_def::VANILLA_CONTROLLER;
    }
    return StationKind_def::INVALID_CONTROLLER;
}

void run_example(unsigned int domain_id, const std::string& station_kind)
{
    StationKind current_station = string_to_stationkind(station_kind);
    std::cout << station_kind << " station starting" << std::endl;
    std::map<StationKind, StationKind> next_station;
    next_station[StationKind_def::COCOA_BUTTER_CONTROLLER] = StationKind_def::SUGAR_CONTROLLER;
    next_station[StationKind_def::SUGAR_CONTROLLER] = StationKind_def::MILK_CONTROLLER;
    next_station[StationKind_def::MILK_CONTROLLER] = StationKind_def::VANILLA_CONTROLLER;
    next_station[StationKind_def::VANILLA_CONTROLLER] = StationKind_def::TEMPERING_CONTROLLER;
    
    // Loads the QoS from the qos_profiles.xml file. 
    dds::core::QosProvider qos_provider("./qos_profiles.xml");

    // A DomainParticipant allows an application to begin communicating in
    // a DDS domain. Typically there is one DomainParticipant per application.
    // Uses TemperingApplication QoS profile to set participant name.
    dds::domain::DomainParticipant participant(
                domain_id,
                qos_provider.participant_qos(
                            "ChocolateFactoryLibrary::TemperingApplication"));

    // A Topic has a name and a datatype. Create Topics.
    // Topic names are constants defined in the IDL file.
    dds::topic::Topic<ChocolateLotState> lot_state_topic(
            participant,
            CHOCOLATE_LOT_STATE_TOPIC);
    std::string filter_value = "'" + station_kind + "'";
    dds::topic::ContentFilteredTopic<ChocolateLotState>
            filtered_lot_state_topic(
                    lot_state_topic,
                    "FilteredLot",
                    dds::topic::Filter(
                            "next_station = %0",
                            { filter_value }));

    // A Publisher allows an application to create one or more DataWriters
    // Create Publisher with default QoS
    dds::pub::Publisher publisher(participant);

    // Create DataWriter of Topic "ChocolateLotState"
    // using ChocolateLotStateProfile QoS profile for State Data
    dds::pub::DataWriter<ChocolateLotState> lot_state_writer(
            publisher,
            lot_state_topic,
            qos_provider.datawriter_qos(
                    "ChocolateFactoryLibrary::ChocolateLotStateProfile"));

    // A Subscriber allows an application to create one or more DataReaders
    dds::sub::Subscriber subscriber(participant);

    // Create DataReader of Topic "ChocolateLotState".
    // using ChocolateLotStateProfile QoS profile for State Data
    dds::sub::DataReader<ChocolateLotState> lot_state_reader(
            subscriber,
            filtered_lot_state_topic,
            qos_provider.datareader_qos(
                    "ChocolateFactoryLibrary::ChocolateLotStateProfile"));

    // Obtain the DataReader's Status Condition
    dds::core::cond::StatusCondition reader_status_condition(lot_state_reader);

    // Contains statuses that entities can be notified about
    using dds::core::status::StatusMask;

    // Enable the 'data available' and 'requested incompatible qos' statuses
    reader_status_condition.enabled_statuses(
            StatusMask::data_available());

    // Associate a handler with the status condition. This will run when the
    // condition is triggered, in the context of the dispatch call (see below)
    reader_status_condition.extensions().handler([&current_station,
                                                  &next_station,
                                                  &lot_state_reader,
                                                  &lot_state_writer]() {
        if ((lot_state_reader.status_changes() & StatusMask::data_available())
                != StatusMask::none()) {
            process_lot(current_station, next_station, lot_state_reader, lot_state_writer);
        }
    });

    // Create a WaitSet and attach the StatusCondition
    dds::core::cond::WaitSet waitset;
    waitset += reader_status_condition;

    while (!shutdown_requested) {
        // Wait for ChocolateLotState
        std::cout << "Waiting for lot" << std::endl;
        waitset.dispatch(dds::core::Duration(10));  // Wait up to 10s for update
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
        run_example(arguments.domain_id, arguments.station_kind);
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
