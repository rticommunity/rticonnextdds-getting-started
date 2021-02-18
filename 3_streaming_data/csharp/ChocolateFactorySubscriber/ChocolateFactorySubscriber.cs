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

using System;
using Omg.Dds.Core;
using Rti.Dds.Core;
using Rti.Dds.Core.Status;
using Rti.Dds.Domain;
using Rti.Dds.Subscription;
using Rti.Dds.Topics;

namespace StreamingData
{
    /// <summary>
    /// Example subscriber application
    /// </summary>
    public class ChocolateFactorySubscriber
    {
        private bool shutdownRequested;

        private static int ProcessData(DataReader<Temperature> reader)
        {
            // Take all samples. Samples are loaned to application, loan is
            // returned when samples.Dispose() is called.
            int samplesRead = 0;
            using (var samples = reader.Take())
            {
                foreach (var sample in samples.ValidData())
                {
                    Console.WriteLine(sample);
                    samplesRead++;
                }
            }

            return samplesRead;
        }

        private void RunExample(int domainId, int sampleCount)
        {
            // A DomainParticipant allows an application to begin communicating in
            // a DDS domain. Typically there is one DomainParticipant per application.
            // DomainParticipant QoS is configured in USER_QOS_PROFILES.xml
            DomainParticipant participant = DomainParticipantFactory.Instance
                .CreateParticipant(domainId);

            // A Topic has a name and a datatype. Create a Topic named
            // "ChocolateTemperature" with type Temperature.
            Topic<Temperature> topic = participant.CreateTopic<Temperature>("ChocolateTemperature");

            // A Subscriber allows an application to create one or more DataReaders
            // Subscriber QoS is configured in USER_QOS_PROFILES.xml
            Subscriber subscriber = participant.CreateSubscriber();

            // This DataReader reads data of type Temperature on Topic
            // "ChocolateTemperature". DataReader QoS is configured in
            // USER_QOS_PROFILES.xml
            DataReader<Temperature> reader = subscriber.CreateDataReader(topic);

            // Obtain the DataReader's Status Condition
            StatusCondition statusCondition = reader.StatusCondition;

            // Enable the 'data available' status.
            statusCondition.EnabledStatuses = StatusMask.DataAvailable;

            // Associate an event handler with the status condition.
            // This will run when the condition is triggered, in the context of
            // the dispatch call (see below)
            int samplesRead = 0;
            statusCondition.Triggered += _ => samplesRead += ProcessData(reader);

            // Create a WaitSet and attach the StatusCondition
            var waitset = new WaitSet();
            waitset.AttachCondition(statusCondition);
            while (samplesRead < sampleCount && !shutdownRequested)
            {
                // Dispatch will call the handlers associated to the WaitSet
                // conditions when they activate
                Console.WriteLine("ChocolateTemperature subscriber sleeping for 4 sec...");
                waitset.Dispatch(Duration.FromSeconds(4));
            }
        }

        /// <summary>
        /// Main function, receiving structured command-line arguments
        /// via the System.Console.DragonFruit package.
        /// For example: dotnet run -- --domain-id 54 --sample-count 5
        /// </summary>
        /// <param name="domainId">The domain ID to create the DomainParticipant</param>
        /// <param name="sampleCount">The number of data samples to publish</param>
        public static void Main(int domainId = 0, int sampleCount = int.MaxValue)
        {
            var example = new ChocolateFactorySubscriber();

            // Setup signal handler
            Console.CancelKeyPress += (_, eventArgs) =>
            {
                Console.WriteLine("Shuting down...");
                eventArgs.Cancel = true; // let the application shutdown gracefully
                example.shutdownRequested = true;
            };

            try
            {
                example.RunExample(domainId, sampleCount);
            }
            catch (Exception ex)
            {
                Console.WriteLine("RunExample exception: " + ex.Message);
                Console.WriteLine(ex.StackTrace);
            }
        }
    }
}
