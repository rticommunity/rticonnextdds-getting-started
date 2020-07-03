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

static int shutdown(
        DDSDomainParticipant *participant,
        const char *shutdown_message,
        int status);

// Structure to hold data for write lot sate thread
struct StartLotThreadData {
    ChocolateLotStateDataWriter *writer;
    unsigned int lots_to_process;
};

void publish_start_lot(StartLotThreadData *thread_data)
{
    ChocolateLotState sample;
    unsigned int lots_to_process = thread_data->lots_to_process;
    ChocolateLotStateDataWriter *writer = thread_data->writer;

    for (unsigned int count = 0; !shutdown_requested && count < lots_to_process;
         count++) {
        // Set the values for a chocolate lot that is going to be sent to wait
        // at the tempering station
        sample.lot_id = count % 100;
        sample.lot_status = WAITING;
        sample.next_station = TEMPERING_CONTROLLER;

        std::cout << std::endl
                  << "Starting lot: " << std::endl
                  << "[lot_id: " << sample.lot_id << " next_station: ";
        print_station_kind(sample.next_station);
        std::cout << "]" << std::endl;

        // Send an update to station that there is a lot waiting for tempering
        DDS_ReturnCode_t retcode = writer->write(sample, DDS_HANDLE_NIL);
        if (retcode != DDS_RETCODE_OK) {
            std::cerr << "write error " << retcode << std::endl;
        }

        // Start a new lot every 10 seconds
        DDS_Duration_t send_period = { 10, 0 };
        NDDSUtility::sleep(send_period);
    }
}

// Process data. Returns number of samples processed.
unsigned int monitor_lot_state(ChocolateLotStateDataReader *lot_state_reader)
{
    ChocolateLotStateSeq data_seq;
    DDS_SampleInfoSeq info_seq;
    unsigned int samples_read = 0;

    // Take available data from DataReader's queue
    DDS_ReturnCode_t retcode = lot_state_reader->take(data_seq, info_seq);
    if (retcode != DDS_RETCODE_OK && retcode != DDS_RETCODE_NO_DATA) {
        std::cerr << "take error " << retcode << std::endl;
        return 0;
    }

    // Iterate over all available data
    for (int i = 0; i < data_seq.length(); ++i) {
        // Check if a sample is an instance lifecycle event
        if (info_seq[i].valid_data) {
            std::cout << "Received lot update:" << std::endl;
            application::print_chocolate_lot_data(&data_seq[i]);
            samples_read++;
        } else {
            // Detect that a lot is complete because the instance is disposed
            if (info_seq[i].instance_state
                    == DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE) {
                ChocolateLotState key_holder;
                // Fills in only the key field values associated with the
                // instance
                lot_state_reader->get_key_value(
                        key_holder,
                        info_seq[i].instance_handle);
                std::cout << "[lot_id: " << key_holder.lot_id
                        << " is completed]" << std::endl;
            }
        }
    }
    // Data sequence was loaned from middleware for performance.
    // Return loan when application is finished with data.
    retcode = lot_state_reader->return_loan(data_seq, info_seq);
    if (retcode != DDS_RETCODE_OK) {
        std::cerr << "return_loan error " << retcode << std::endl;
    }

    return samples_read;
}

// Monitor the chocolate temperature
void monitor_temperature(TemperatureDataReader *temperature_reader)
{
    TemperatureSeq data_seq;
    DDS_SampleInfoSeq info_seq;

    // Take available data from DataReader's queue
    DDS_ReturnCode_t retcode = temperature_reader->take(data_seq, info_seq);

    if (retcode != DDS_RETCODE_OK && retcode != DDS_RETCODE_NO_DATA) {
        std::cerr << "take error " << retcode << std::endl;
        return;
    }

    // Iterate over all available data
    for (int i = 0; i < data_seq.length(); ++i) {
        // Check if a sample is an instance lifecycle event
        if (!info_seq[i].valid_data) {
            std::cout << "Received instance state notification" << std::endl;
            continue;
        }
        // Print data
        if (data_seq[i].degrees > 32) {
            std::cout << "Temperature high: ";
            TemperatureTypeSupport::print_data(&data_seq[i]);
        }
    }
    // Data sequence was loaned from middleware for performance.
    // Return loan when application is finished with data.
    temperature_reader->return_loan(data_seq, info_seq);
}

int run_example(unsigned int domain_id, unsigned int sample_count)
{
    // Exercise #1.1: Load QoS file

    // Connext DDS Setup
    // -----------------
    // A DomainParticipant allows an application to begin communicating in
    // a DDS domain. Typically there is one DomainParticipant per application.
    // DomainParticipant QoS is configured in USER_QOS_PROFILES.xml
    // Exercise #1.2: Load DomainParticipant QoS profile
    DDSDomainParticipant *participant =
            DDSTheParticipantFactory->create_participant(
                    domain_id,
                    DDS_PARTICIPANT_QOS_DEFAULT,
                    NULL /* listener */,
                    DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        shutdown(participant, "create_participant error", EXIT_FAILURE);
    }

    // Create Topics

    // Register the datatype to use when creating the Topic
    const char *lot_state_type_name =
            ChocolateLotStateTypeSupport::get_type_name();
    DDS_ReturnCode_t retcode = ChocolateLotStateTypeSupport::register_type(
            participant,
            lot_state_type_name);
    if (retcode != DDS_RETCODE_OK) {
        shutdown(participant, "register_type error", EXIT_FAILURE);
    }
    // A Topic has a name and a datatype. Create a Topic called
    // "ChocolateLotState" with your registered data type
    DDSTopic *lot_state_topic = participant->create_topic(
            CHOCOLATE_LOT_STATE_TOPIC,
            lot_state_type_name,
            DDS_TOPIC_QOS_DEFAULT,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (lot_state_topic == NULL) {
        shutdown(participant, "create_topic error", EXIT_FAILURE);
    }
    // Register the datatype to use when creating the Topic
    const char *temperature_type_name =
            TemperatureTypeSupport::get_type_name();
    retcode = TemperatureTypeSupport::register_type(
            participant,
            temperature_type_name);
    if (retcode != DDS_RETCODE_OK) {
        shutdown(participant, "register_type error", EXIT_FAILURE);
    }
    // A Topic has a name and a datatype. Create a Topic called
    // "ChocolateTemperature" with your registered data type
    DDSTopic *temperature_topic = participant->create_topic(
            CHOCOLATE_TEMPERATURE_TOPIC,
            temperature_type_name,
            DDS_TOPIC_QOS_DEFAULT,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (temperature_topic == NULL) {
        shutdown(participant, "create_topic error", EXIT_FAILURE);
    }

    // Create Publisher and DataWriter

    // A Publisher allows an application to create one or more DataWriters
    // Publisher QoS is configured in USER_QOS_PROFILES.xml
    DDSPublisher *publisher = participant->create_publisher(
            DDS_PUBLISHER_QOS_DEFAULT,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (publisher == NULL) {
        return shutdown(participant, "create_publisher error", EXIT_FAILURE);
    }

    // This DataWriter writes data on Topic "ChocolateLotState"
    // DataWriter QoS is configured in USER_QOS_PROFILES.xml
    // Exercise #4.1: Load ChocolateLotState DataWriter QoS profile after
    // debugging incompatible QoS    
    DDSDataWriter *generic_lot_state_writer = publisher->create_datawriter(
            lot_state_topic,
            DDS_DATAWRITER_QOS_DEFAULT,
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

    // Create Subscriber and DataReaders

    // A Subscriber allows an application to create one or more DataReaders
    // Subscriber QoS is configured in USER_QOS_PROFILES.xml
    DDSSubscriber *subscriber = participant->create_subscriber(
            DDS_SUBSCRIBER_QOS_DEFAULT,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (subscriber == NULL) {
        shutdown(participant, "create_subscriber error", EXIT_FAILURE);
    }

    // This DataReader reads data of type ChocolateLotState on Topic
    // "ChocolateLotState". DataReader QoS is configured in
    // USER_QOS_PROFILES.xml
    // Exercise #1.3: Update the lot_state_reader to use correct QoS    
    DDSDataReader *lot_state_generic_reader = subscriber->create_datareader(
            lot_state_topic,
            DDS_DATAREADER_QOS_DEFAULT,
            NULL,
            DDS_STATUS_MASK_NONE);
    if (lot_state_generic_reader == NULL) {
        shutdown(participant, "create_datareader error", EXIT_FAILURE);
    }

    // Get status condition: Each entity has a Status Condition, which
    // gets triggered when a status becomes true
    DDSStatusCondition *lot_state_status_condition =
            lot_state_generic_reader->get_statuscondition();
    if (lot_state_status_condition == NULL) {
        shutdown(participant, "get_statuscondition error", EXIT_FAILURE);
    }

    // Enable only the status we are interested in:
    //   DDS_DATA_AVAILABLE_STATUS
    retcode = lot_state_status_condition->set_enabled_statuses(
            DDS_DATA_AVAILABLE_STATUS);
    if (retcode != DDS_RETCODE_OK) {
        shutdown(participant, "set_enabled_statuses error", EXIT_FAILURE);
    }

    // This DataReader reads data of type Temperature on Topic
    // "ChocolateTemperature". DataReader QoS is configured in
    // USER_QOS_PROFILES.xml
    // Exercise #1.4: Update the lot_state_reader to use correct QoS    
    DDSDataReader *temperature_generic_reader = subscriber->create_datareader(
            temperature_topic,
            DDS_DATAREADER_QOS_DEFAULT,
            NULL,
            DDS_STATUS_MASK_NONE);
    if (temperature_generic_reader == NULL) {
        shutdown(participant, "create_datareader error", EXIT_FAILURE);
    }

    // Get status condition: Each entity has a Status Condition, which
    // gets triggered when a status becomes true
    DDSStatusCondition *temperature_status_condition =
            temperature_generic_reader->get_statuscondition();
    if (temperature_status_condition == NULL) {
    shutdown(participant, "get_statuscondition error", EXIT_FAILURE);
    }

    // Enable only the status we are interested in:
    //   DDS_DATA_AVAILABLE_STATUS
    retcode = temperature_status_condition->set_enabled_statuses(
            DDS_DATA_AVAILABLE_STATUS);
    if (retcode != DDS_RETCODE_OK) {
        shutdown(participant, "set_enabled_statuses error", EXIT_FAILURE);
    }

    // Create the WaitSet and attach the Status Condition to it. The WaitSet
    // will be woken when the condition is triggered.
    DDSWaitSet waitset;
    retcode = waitset.attach_condition(lot_state_status_condition);
    if (retcode != DDS_RETCODE_OK) {
        shutdown(participant, "attach_condition error", EXIT_FAILURE);
    }
    // Temperature condition
    retcode = waitset.attach_condition(temperature_status_condition);
    if (retcode != DDS_RETCODE_OK) {
        shutdown(participant, "attach_condition error", EXIT_FAILURE);
    }
    // A narrow is a cast from a generic DataReader to one that is specific
    // to your type. Use the type specific DataReader to read data
    ChocolateLotStateDataReader *lot_state_reader =
            ChocolateLotStateDataReader::narrow(lot_state_generic_reader);
    if (lot_state_reader == NULL) {
        shutdown(participant, "DataReader narrow error", EXIT_FAILURE);
    }
    TemperatureDataReader *temperature_reader =
            TemperatureDataReader::narrow(temperature_generic_reader);
    if (temperature_reader == NULL) {
        shutdown(participant, "DataReader narrow error", EXIT_FAILURE);
    }

    // Create a thread to periodically start new chocolate lots
    StartLotThreadData thread_data;
    thread_data.writer = lot_state_writer;
    thread_data.lots_to_process = sample_count;
    OSThread thread((ThreadFunction) publish_start_lot, (void *) &thread_data);
    thread.run();

    // Main loop. Wait for data to arrive, and process when it arrives.
    // ----------------------------------------------------------------
    unsigned int samples_read = 0;
    while (!shutdown_requested && samples_read < sample_count) {
        DDSConditionSeq active_conditions_seq;

        // wait() blocks execution of the thread until one or more attached
        // Conditions become true, or until a user-specified timeout expires.
        DDS_Duration_t wait_timeout = { 15, 0 };
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
        DDS_StatusMask triggeredmask = lot_state_reader->get_status_changes();

        // If the ChocolateLotState DataReader received DATA_AVAILABLE event
        // notification
        if (triggeredmask & DDS_DATA_AVAILABLE_STATUS) {
            samples_read += monitor_lot_state(lot_state_reader);
        }
        // Check if the temperature DataReader received DATA_AVAILABLE event
        // notification
        triggeredmask = temperature_reader->get_status_changes();
        if (triggeredmask & DDS_DATA_AVAILABLE_STATUS) {
            monitor_temperature(temperature_reader);
        }
    }

    // Cleanup
    // -------
    thread.join();
    // Delete all entities (DataReader, Topic, Subscriber, DomainParticipant)
    return shutdown(participant, "shutting down", 0);
}

// Delete all entities
static int shutdown(
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
            std::cerr << "delete_contained_entities error" << retcode
                      << std::endl;
            status = EXIT_FAILURE;
        }

        retcode = DDSTheParticipantFactory->delete_participant(participant);
        if (retcode != DDS_RETCODE_OK) {
            std::cerr << "delete_participant error" << retcode << std::endl;
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

    int status = run_example(arguments.domain_id, arguments.sample_count);

    // Releases the memory used by the participant factory.  Optional at
    // application shutdown
    DDS_ReturnCode_t retcode = DDSDomainParticipantFactory::finalize_instance();
    if (retcode != DDS_RETCODE_OK) {
        std::cerr << "finalize_instance error" << retcode << std::endl;
        status = EXIT_FAILURE;
    }

    return status;
}
