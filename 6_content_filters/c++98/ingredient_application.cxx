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
#include <stdio.h>
#include <stdlib.h>

#include "chocolate_factory.h"
#include "chocolate_factorySupport.h"
#include "ndds/ndds_cpp.h"
#include "application.h"

using namespace application;

// Ingredient application:
// 1) Subscribes to the lot state
// 2) "Processes" the lot. (In this example, that means sleep for a time)
// 3) After "processing" the lot, publishes the lot state


static int shutdown(
        DDSDomainParticipant *participant,
        const char *shutdown_message,
        int status);

void process_lot(
        const StationKind station_kind,
        ChocolateLotStateDataReader *lot_state_reader,
        ChocolateLotStateDataWriter *lot_state_writer)
{
    // Take all samples.  Samples are loaned to application, loan is
    // returned when LoanedSamples destructor called.
    ChocolateLotStateSeq data_seq;
    DDS_SampleInfoSeq info_seq;

    // Take available data from DataReader's queue
    DDS_ReturnCode_t retcode = lot_state_reader->take(data_seq, info_seq);
    
    if (retcode != DDS_RETCODE_OK && retcode != DDS_RETCODE_NO_DATA) {
        std::cerr << "take error " << retcode << std::endl;
        return;
    }

    // Process lots waiting for ingredients
    for (int i = 0; i < data_seq.length(); ++i) {
        // Check if a sample is an instance lifecycle event
        if (info_seq[i].valid_data) {
            // No need to check that this is the next station: content filter
            // ensures that the reader only receives lots with
            // next_station == this station.
            std::cout << "Processing lot #" << data_seq[i].lot_id << std::endl;

            // Send an update that the this station is processing lot
            ChocolateLotState updated_state(data_seq[i]);
            updated_state.lot_status = PROCESSING;
            updated_state.next_station = INVALID_CONTROLLER;
            updated_state.station = station_kind;
            DDS_ReturnCode_t retcode =
                    lot_state_writer->write(updated_state, DDS_HANDLE_NIL);
            if (retcode != DDS_RETCODE_OK) {
                std::cerr << "write error " << retcode << std::endl;
            }

            // "Processing" the lot.
            DDS_Duration_t processing_time = { 5, 0 };
            NDDSUtility::sleep(processing_time);

            // Send an update that this station is done processing the lot
            updated_state.lot_status = COMPLETED;
            updated_state.next_station = (StationKind)(station_kind + 1);
            updated_state.station = station_kind;
            retcode = lot_state_writer->write(updated_state, DDS_HANDLE_NIL);
            if (retcode != DDS_RETCODE_OK) {
                std::cerr << "write error " << retcode << std::endl;
            }

        } else {
            // Received instance state notification
            continue;
        }
    }

    // Data sequence was loaned from middleware for performance.
    // Return loan when application is finished with data.
    retcode = lot_state_reader->return_loan(data_seq, info_seq);
    if (retcode != DDS_RETCODE_OK) {
        std::cerr << "return_loan error " << retcode << std::endl;
    }
}

void on_requested_incompatible_qos(DDSDataReader *reader)
{
    DDS_RequestedIncompatibleQosStatus status;
    reader->get_requested_incompatible_qos_status(status);

    std::cout << "Discovered DataWriter with incompatible policy: ";
    if (status.last_policy_id == DDS_RELIABILITY_QOS_POLICY_ID) {
        std::cout << "Reliability" << std::endl;
    }
    if (status.last_policy_id == DDS_DURABILITY_QOS_POLICY_ID) {
        std::cout << "Durability" << std::endl;
    }    
}

StationKind string_to_stationkind(const std::string& station_kind)
{
    if (station_kind == "SUGAR_CONTROLLER") {
        return SUGAR_CONTROLLER;
    } else if (station_kind == "COCOA_BUTTER_CONTROLLER") {
        return COCOA_BUTTER_CONTROLLER;
    } else if (station_kind == "MILK_CONTROLLER") {
        return MILK_CONTROLLER;
    } else if (station_kind == "VANILLA_CONTROLLER") {
        return VANILLA_CONTROLLER;
    }
    return INVALID_CONTROLLER;
}

int run_example(unsigned int domain_id, const std::string& station_kind)
{
    StationKind current_station = string_to_stationkind(station_kind);
    std::cout << station_kind << " station starting" << std::endl;

    // Load XML QoS from a specific file
    DDSDomainParticipantFactory *factory =
	        DDSDomainParticipantFactory::get_instance();
    DDS_DomainParticipantFactoryQos factoryQos;
    DDS_ReturnCode_t retcode = factory->get_qos(factoryQos);
    if (retcode != DDS_RETCODE_OK) {
        return shutdown(NULL, "get_qos error", EXIT_FAILURE);
    }
    const char *url_profiles[1] = { "qos_profiles.xml" };
    factoryQos.profile.url_profile.from_array(url_profiles, 1);
    factory->set_qos(factoryQos);

    // Connext DDS setup
    // -----------------
    // A DomainParticipant allows an application to begin communicating in
    // a DDS domain. Typically there is one DomainParticipant per application.
    // Uses IngredientApplication QoS profile to set participant name.
    DDSDomainParticipant *participant =
            DDSTheParticipantFactory->create_participant_with_profile(
                    domain_id,
                    "ChocolateFactoryLibrary",
                    "IngredientApplication",
                    NULL /* listener */,
                    DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        return shutdown(participant, "create_participant error", EXIT_FAILURE);
    }

    // Create Topics

    // Register the datatype to use when creating the Topic
    const char *lot_state_type_name =
            ChocolateLotStateTypeSupport::get_type_name();
    retcode = ChocolateLotStateTypeSupport::register_type(
            participant,
            lot_state_type_name);
    if (retcode != DDS_RETCODE_OK) {
        return shutdown(participant, "register_type error", EXIT_FAILURE);
    }

    // A Topic has a name and a datatype. Create a Topic called
    // "ChocolateLotState" with your registered ChocolateLotState data type
    DDSTopic *lot_state_topic = participant->create_topic(
            CHOCOLATE_LOT_STATE_TOPIC,
            lot_state_type_name,
            DDS_TOPIC_QOS_DEFAULT,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (lot_state_topic == NULL) {
        return shutdown(participant, "create_topic error", EXIT_FAILURE);
    }
    // Create content-filtered Topic
    DDS_StringSeq parameters(1);
    parameters.length(1);
    char filter_value[256];
    snprintf(filter_value, 256, "\'%s\'", station_kind.c_str());
    std::cout << "about to set filter_value" << std::endl;
    // String sequence owns memory for the strings it contains, must allocate
    parameters[0] = DDS_String_dup(filter_value);
    std::cout << "done" << std::endl;
    DDSContentFilteredTopic *filtered_lot_state_topic =
            participant->create_contentfilteredtopic(
                    "FilteredLotState",
                    lot_state_topic,
                    "next_station = %0",
                    parameters);

    // Create Publisher and DataWriters

    // A Publisher allows an application to create one or more DataWriters
    // Publisher QoS is configured with default QoS
    DDSPublisher *publisher = participant->create_publisher(
            DDS_PUBLISHER_QOS_DEFAULT,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (publisher == NULL) {
        return shutdown(participant, "create_publisher error", EXIT_FAILURE);
    }

    // This DataWriter writes data on Topic "ChocolateLotState". DataWriter
    // QoS is configured using ChocolateLotStateProfile QoS profile
    DDSDataWriter *generic_lot_state_writer = 
            publisher->create_datawriter_with_profile(
                    lot_state_topic,
                    "ChocolateFactoryLibrary",
                    "ChocolateLotStateProfile",
                    NULL /* listener */,
                    DDS_STATUS_MASK_NONE);
    if (generic_lot_state_writer == NULL) {
        return shutdown(participant, "create_datawriter error", EXIT_FAILURE);
    }

    // A narrow is a cast from a generic DataWriter to one that is specific
    // to your type. Use the type specific DataWriter to write()
    ChocolateLotStateDataWriter *lot_state_writer =
            ChocolateLotStateDataWriter::narrow(generic_lot_state_writer);
    if (lot_state_writer == NULL) {
        return shutdown(participant, "DataWriter narrow error", EXIT_FAILURE);
    }

    // Create Subscriber and DataReader

    // A Subscriber allows an application to create one or more DataReaders
    // Subscriber QoS is configured with default QoS
    DDSSubscriber *subscriber = participant->create_subscriber(
            DDS_SUBSCRIBER_QOS_DEFAULT,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (subscriber == NULL) {
        shutdown(participant, "create_subscriber error", EXIT_FAILURE);
    }

    // This DataReader reads data of type ChocolateLotState on Topic
    // "ChocolateLotState" using ChocolateLotStateProfile QoS profile
    DDSDataReader *generic_lot_state_reader =
            subscriber->create_datareader_with_profile(
                    lot_state_topic,
                    "ChocolateFactoryLibrary",
                    "ChocolateLotStateProfile",
                    NULL,
                    DDS_STATUS_MASK_NONE);
    if (generic_lot_state_reader == NULL) {
        shutdown(participant, "create_datareader error", EXIT_FAILURE);
    }
    // Get status condition: Each entity has a Status Condition, which
    // gets triggered when a status becomes true
    DDSStatusCondition *status_condition =
            generic_lot_state_reader->get_statuscondition();
    if (status_condition == NULL) {
        shutdown(participant, "get_statuscondition error", EXIT_FAILURE);
    }

    // Enable only the statuses we are interested in:
    //   DDS_DATA_AVAILABLE_STATUS, DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS
    retcode = status_condition->set_enabled_statuses(
            DDS_DATA_AVAILABLE_STATUS | DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS);
    if (retcode != DDS_RETCODE_OK) {
        shutdown(participant, "set_enabled_statuses error", EXIT_FAILURE);
    }

    // Create the WaitSet and attach the Status Condition to it. The WaitSet
    // will be woken when the condition is triggered.
    DDSWaitSet waitset;
    retcode = waitset.attach_condition(status_condition);
    if (retcode != DDS_RETCODE_OK) {
        shutdown(participant, "attach_condition error", EXIT_FAILURE);
    }

    // A narrow is a cast from a generic DataReader to one that is specific
    // to your type. Use the type specific DataReader to read data
    ChocolateLotStateDataReader *lot_state_reader =
            ChocolateLotStateDataReader::narrow(generic_lot_state_reader);
    if (lot_state_reader == NULL) {
        shutdown(participant, "DataReader narrow error", EXIT_FAILURE);
    }

    // Main loop, wait for lots
    // ------------------------
    while (!shutdown_requested) {
        // Wait for ChocolateLotState
        std::cout << "Waiting for lot" << std::endl;

        // wait() blocks execution of the thread until one or more attached
        // Conditions become true, or until a user-specified timeout expires.
        DDSConditionSeq active_conditions_seq;
        DDS_Duration_t wait_timeout = { 10, 0 };
        retcode = waitset.wait(active_conditions_seq, wait_timeout);

        // You get a timeout if no conditions were triggered before the timeout
        if (retcode == DDS_RETCODE_TIMEOUT) {
            std::cout << "Wait timed out after 10 seconds." << std::endl;
            continue;
        } else if (retcode != DDS_RETCODE_OK) {
            std::cerr << "wait returned error: " << retcode << std::endl;
            break;
        }

        // Get the status changes to check which status condition
        // triggered the WaitSet to wake
        DDS_StatusMask triggered_mask = lot_state_reader->get_status_changes();

        // If the status is "Data Available"
        if (triggered_mask & DDS_DATA_AVAILABLE_STATUS) {
            process_lot(current_station, lot_state_reader, lot_state_writer);
        }
        if (triggered_mask & DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS) {
            on_requested_incompatible_qos(lot_state_reader);
        }
    }

    // Cleanup
    // -------
    // Delete all entities (DataWriter, Topic, Publisher, DomainParticipant)
    return shutdown(participant, "shutting down", EXIT_SUCCESS);
}

// Delete all entities
int shutdown(
        DDSDomainParticipant *participant,
        const char *shutdown_message,
        int status)
{
    DDS_ReturnCode_t retcode;

    std::cout << shutdown_message << std::endl;

    if (participant != NULL) {
        // This includes everything created by this Participant, including
        // DataWriters, Topics, Publishers. (and Subscribers and DataReaders)
        retcode = participant->delete_contained_entities();
        if (retcode != DDS_RETCODE_OK) {
            std::cerr << "delete_contained_entities error " << retcode
                      << std::endl;
            status = EXIT_FAILURE;
        }

        retcode = DDSTheParticipantFactory->delete_participant(participant);
        if (retcode != DDS_RETCODE_OK) {
            std::cerr << "delete_participant error " << retcode << std::endl;
            status = EXIT_FAILURE;
        }
    }

    return status;
}

int main(int argc, char *argv[])
{
    // Parse arguments and handle control-C
    ApplicationArguments arguments;
    parse_arguments(arguments, argc, argv);
    if (arguments.parse_result == PARSE_RETURN_EXIT) {
        return EXIT_SUCCESS;
    } else if (arguments.parse_result == PARSE_RETURN_FAILURE) {
        return EXIT_FAILURE;
    }
    setup_signal_handlers();

    // Sets Connext verbosity to help debugging
    NDDSConfigLogger::get_instance()->set_verbosity(arguments.verbosity);

    int status = run_example(arguments.domain_id, arguments.station_kind);

    // Releases the memory used by the participant factory.  Optional at
    // application shutdown
    DDS_ReturnCode_t retcode = DDSDomainParticipantFactory::finalize_instance();
    if (retcode != DDS_RETCODE_OK) {
        std::cerr << "finalize_instance error " << retcode << std::endl;
        status = EXIT_FAILURE;
    }

    return status;
}
