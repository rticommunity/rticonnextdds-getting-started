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

import com.rti.dds.domain.DomainParticipant;
import com.rti.dds.domain.DomainParticipantFactory;
import com.rti.dds.infrastructure.InstanceHandle_t;
import com.rti.dds.infrastructure.RETCODE_ERROR;
import com.rti.dds.infrastructure.StatusKind;
import com.rti.dds.publication.Publisher;
import com.rti.dds.topic.Topic;
import com.rti.ndds.config.LogVerbosity;
import com.rti.ndds.config.Logger;

// ===========================================================================

public class HelloMessagePublisher extends Application {
    
    
    public static void main(String[] args) {
        setUpLogging();
                        
        try { 

            // Parse arguments passed to application
            parseArguments(args);
            if (ApplicationArguments.runApplication.get() == true) {
                // If the application is running, set a shutdown handler and
                // set Connext logging verbosity
                handleShutdown();
                setVerbosity();

                runExample();
            }
        } catch (Exception e) {
            logger.severe("Exception: " + e.getLocalizedMessage());
        }
    }


    private HelloMessagePublisher() {
        super();
    }

    private static void runExample() {

        DomainParticipant participant = null;
        try {
            // Connext DDS setup
            // -----------------
            // A DomainParticipant allows an application to begin communicating 
            // in a DDS domain. Typically there is one DomainParticipant per 
            // application. DomainParticipant QoS is configured in 
            // USER_QOS_PROFILES.xml
            participant = DomainParticipantFactory.TheParticipantFactory
                                  .create_participant(
                                          ApplicationArguments.domainId.get(),
                                          DomainParticipantFactory
                                                  .PARTICIPANT_QOS_DEFAULT,
                                          null,  // listener
                                          StatusKind.STATUS_MASK_NONE);
            if (participant == null) {
                throw new RETCODE_ERROR("create_participant error");
            }        

            // A Publisher allows an application to create one or more 
            // DataWriters. Publisher QoS is configured in USER_QOS_PROFILES.xml
            Publisher publisher = participant.create_publisher(
                    DomainParticipant.PUBLISHER_QOS_DEFAULT,
                    null,  // listener
                    StatusKind.STATUS_MASK_NONE);
            if (publisher == null) {
                throw new RETCODE_ERROR("create_publisher error");
            }                   

            // Register the datatype to use when creating the Topic
            String typeName = HelloMessageTypeSupport.get_type_name();
            HelloMessageTypeSupport.register_type(participant, typeName);

            // A Topic has a name and a datatype. Create a Topic called
            // "Example HelloMessage" with your registered data type
            Topic topic = participant.create_topic(
                    "Example HelloMessage",
                    typeName,
                    DomainParticipant.TOPIC_QOS_DEFAULT,
                    null,  // listener
                    StatusKind.STATUS_MASK_NONE);
            if (topic == null) {
                throw new RETCODE_ERROR("create_topic error");                
            }           

            // This DataWriter writes data on Topic "Example HelloMessage"
            // DataWriter QoS is configured in USER_QOS_PROFILES.xml
            HelloMessageDataWriter writer =
                    (HelloMessageDataWriter)publisher.create_datawriter(
                            topic,
                            Publisher.DATAWRITER_QOS_DEFAULT,
                            null,  // listener
                            StatusKind.STATUS_MASK_NONE);
            if (writer == null) {
                throw new RETCODE_ERROR("create_datawriter error");
            }           

            // Create data sample for writing
            HelloMessage sample = new HelloMessage();

            // Main loop, write data
            // ---------------------
            for (int count = 0; 
                 shouldRun.get()
                        && ((ApplicationArguments.sampleCount.get() == 0)
                        || (count < ApplicationArguments.sampleCount.get()));
                 ++count) {
                logger.info("Writing HelloMessage, count " + count);

                // Modify the data to be written here

                // Write data
                writer.write(sample, InstanceHandle_t.HANDLE_NIL);
                try {
                    Thread.sleep(4000);  // 4 seconds
                } catch (InterruptedException ex) {
                    // Stop writing
                    break;
                }
            }

        } finally {

            // Shutdown
            // --------
            if (participant != null) {
                participant.delete_contained_entities();

                DomainParticipantFactory.TheParticipantFactory
                        .delete_participant(participant);
                                }
                // Optional at shutdown
                DomainParticipantFactory.finalize_instance();  
        }
    }
    
    // Sets Connext verbosity to help debugging
    public static void setVerbosity() {
        
        switch (ApplicationArguments.verbosity.get()) {
            case 0:
                Logger.get_instance().set_verbosity(
                        LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_SILENT);
                    break;
            case 1:
                Logger.get_instance().set_verbosity(
                        LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_ERROR);
                    break;
            case 2:
                Logger.get_instance().set_verbosity(
                        LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_WARNING);
                    break;
            case 3:
                Logger.get_instance().set_verbosity(
                        LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
                    break;
            default: 
                Logger.get_instance().set_verbosity(
                        LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_ERROR);
                break;
        }

    }
}
