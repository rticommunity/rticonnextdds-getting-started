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
/* hello_world_subscriber.cxx

A subscription example of type HelloMessage

To test it, follow these steps:


(1) Compile this file and the example publication.

(2) Start the subscription

(3) Start the publication

(4) [Optional] Specify the list of discovery initial peers and
multicast receive addresses via an environment variable or a file
(in the current working directory) called NDDS_DISCOVERY_PEERS.

You can run any number of publisher and subscriber programs, and can
add and remove them dynamically from the domain.
*/

#include <stdio.h>
#include <stdlib.h>

#include "hello_world.h"
#include "hello_worldSupport.h"
#include "ndds/ndds_cpp.h"

static int shutdown(
        DDSDomainParticipant *participant,
        const char *shutdown_message,
        int status);

// Process data. Returns number of samples processed.
int process_data(HelloMessageDataReader *HelloMessage_reader)
{
    HelloMessageSeq data_seq;
    DDS_SampleInfoSeq info_seq;
    int count = 0;

    // Take available data from DataReader's queue
    DDS_ReturnCode_t retcode = DDS_RETCODE_OK;
    while (retcode != DDS_RETCODE_NO_DATA) {
        retcode = HelloMessage_reader->take(
                data_seq,
                info_seq,
                DDS_LENGTH_UNLIMITED,
                DDS_ANY_SAMPLE_STATE,
                DDS_ANY_VIEW_STATE,
                DDS_ANY_INSTANCE_STATE);

        // Iterate over all available data
        for (int j = 0; j < data_seq.length(); ++j) {
            // Check if a sample is an instance lifecycle event
            if (!info_seq[j].valid_data) {
                printf("Received instance state notification\n");
                continue;
            }
            // Print data
            HelloMessageTypeSupport::print_data(&data_seq[j]);
            count++;
        }
        // Data sequence was loaned from middleware for performance.
        // Return loan when application is finished with data.
        HelloMessage_reader->return_loan(data_seq, info_seq);
    }
    
    return count;
}

int subscriber_run(int domain_id, int sample_count)
{
    // Connext DDS Setup
    // -----------------
    // A DomainParticipant allows an application to begin communicating in
    // a DDS domain. Typically there is one DomainParticipant per application.
    // DomainParticipant QoS is configured in USER_QOS_PROFILES.xml
    DDSDomainParticipant *participant =
            DDSTheParticipantFactory->create_participant(
                    domain_id,
                    DDS_PARTICIPANT_QOS_DEFAULT,
                    NULL /* listener */,
                    DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        shutdown(participant, "create_participant error", -1);
    }

    // A Subscriber allows an application to create one or more DataReaders
    // Subscriber QoS is configured in USER_QOS_PROFILES.xml
    DDSSubscriber *subscriber = participant->create_subscriber(
            DDS_SUBSCRIBER_QOS_DEFAULT,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (subscriber == NULL) {
        shutdown(participant, "create_subscriber error", -1);
    }

    // Register the datatype to use when creating the Topic
    const char *type_name = HelloMessageTypeSupport::get_type_name();
    DDS_ReturnCode_t retcode =
            HelloMessageTypeSupport::register_type(participant, type_name);
    if (retcode != DDS_RETCODE_OK) {
        shutdown(participant, "register_type error", -1);
    }

    // A Topic has a name and a datatype. Create a Topic called
    // "Example HelloMessage" with your registered data type
    DDSTopic *topic = participant->create_topic(
            "Example HelloMessage",
            type_name,
            DDS_TOPIC_QOS_DEFAULT,
            NULL /* listener */,
            DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        shutdown(participant, "create_topic error", -1);
    }

    // This DataReader reads data on Topic "Example HelloMessage"
    // DataReader QoS is configured in USER_QOS_PROFILES.xml
    DDSDataReader *reader = subscriber->create_datareader(
            topic,
            DDS_DATAREADER_QOS_DEFAULT,
            NULL,
            DDS_STATUS_MASK_NONE);
    if (reader == NULL) {
        shutdown(participant, "create_datareader error", -1);
    }

    // Get status condition: Each entity has a Status Condition, which
    // gets triggered when a status becomes true
    DDSStatusCondition *status_condition = reader->get_statuscondition();
    if (status_condition == NULL) {
        shutdown(participant, "get_statuscondition error", -1);
    }

    // Enable only the status we are interested in:
    //   DDS_DATA_AVAILABLE_STATUS
    retcode = status_condition->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
    if (retcode != DDS_RETCODE_OK) {
        shutdown(participant, "set_enabled_statuses error", -1);
    }

    // Create the WaitSet and attach the Status Condition to it. The WaitSet
    // will be woken when the condition is triggered.
    DDSWaitSet *waitset = new DDSWaitSet();
    retcode = waitset->attach_condition(status_condition);
    if (retcode != DDS_RETCODE_OK) {
        shutdown(participant, "attach_condition error", -1);
    }

    // A narrow is a cast from a generic DataReader to one that is specific
    // to your type. Use the type specific DataReader to read data
    HelloMessageDataReader *HelloMessage_reader =
            HelloMessageDataReader::narrow(reader);
    if (HelloMessage_reader == NULL) {
        shutdown(participant, "DataReader narrow error", -1);
    }

    // Main loop. Wait for data to arrive, and process when it arrives.
    // ----------------------------------------------------------------
    int count = 0;
    while (count < sample_count || sample_count == 0) {
        DDSConditionSeq active_conditions_seq;

        // wait() blocks execution of the thread until one or more attached
        // Conditions become true, or until a user-specified timeout expires.
        DDS_Duration_t wait_timeout = { 4, 0 };
        retcode = waitset->wait(active_conditions_seq, wait_timeout);

        // You get a timeout if no conditions were triggered before the timeout
        if (retcode == DDS_RETCODE_TIMEOUT) {
            printf("Wait timed out. No conditions were triggered.\n");
            continue;
        } else if (retcode != DDS_RETCODE_OK) {
            printf("wait returned error: %d\n", retcode);
            break;
        }

        // Get the status changes to check which status condition
        // triggered the the WaitSet to wake
        DDS_StatusMask triggeredmask =
                HelloMessage_reader->get_status_changes();

        // If the status is "Data Available"
        if (triggeredmask & DDS_DATA_AVAILABLE_STATUS) {
            count += process_data(HelloMessage_reader);
        }
    }

    // Cleanup
    // -------
    delete waitset;

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

    printf("%s\n", shutdown_message);

    if (participant != NULL) {
        // This includes everything created by this Participant, including
        // DataWriters, Topics, Publishers. (and Subscribers and DataReaders)
        retcode = participant->delete_contained_entities();
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "delete_contained_entities error %d\n", retcode);
            status = -1;
        }

        retcode = DDSTheParticipantFactory->delete_participant(participant);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "delete_participant error %d\n", retcode);
            status = -1;
        }
    }

    // Releases the memory used by the participant factory.
    retcode = DDSDomainParticipantFactory::finalize_instance();
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "finalize_instance error %d\n", retcode);
        status = -1;
    }

    return status;
}

int main(int argc, char *argv[])
{

    int domain_id = 0;
    int sample_count = 0;  // infinite loop

    if (argc >= 2) {
        domain_id = atoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = atoi(argv[2]);
    }

    // Uncomment this to turn on additional logging
    // NDDSConfigLogger::get_instance()->
    // set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API,
    // NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);

    return subscriber_run(domain_id, sample_count);

}
