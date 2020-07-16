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
using Rti.Types.Dynamic;

namespace HelloWorld
{
    /// <summary>
    /// Example subscriber application
    /// </summary>
    public static class HelloWorldSubscriber
    {
        private static int ProcessData(DataReader<DynamicData> reader)
        {
            // Take all samples. Samples are loaned to application, loan is
            // returned when samples is Disposed.
            int samplesRead = 0;
            using var samples = reader.Take();
            foreach (var sample in samples.ValidData)
            {
                Console.WriteLine($"Received: {sample}");
                samplesRead++;
            }

            return samplesRead;
        }

        /// <summary>
        /// Main function, receiving structured command-line arguments, for example:
        /// dotnet run -- --domain-id 54 --sample-count 5
        /// </summary>
        /// <param name="domainId">The domain ID to create the DomainParticipant</param>
        /// <param name="sampleCount">The number of data samples to receive before exiting</param>
        public static void Main(int domainId = 0, int sampleCount = 10)
        {
            // A DomainParticipant allows an application to begin communicating in
            // a DDS domain. Typically there is one DomainParticipant per application.
            // DomainParticipant QoS is configured in USER_QOS_PROFILES.xml
            //
            // A participant needs to be Disposed to release middleware resources.
            // The 'using' keyword indicates that it will be Disposed when this
            // scope ends.
            using DomainParticipant participant = DomainParticipantFactory.Instance
                .CreateParticipant(domainId);

            // A Topic has a name and a datatype. Create dynamically-typed
            // Topic named "HelloWorld Topic" with the type definition of
            // "HelloWorld" in hell_world.xml. To get the type we use a QosProvider
            var provider = new QosProvider("../hello_world.xml");
            Topic<DynamicData> topic = participant.CreateTopic(
                "Example HelloWorld",
                provider.GetType("HelloWorld"));

            // A Subscriber allows an application to create one or more DataReaders
            // Subscriber QoS is configured in USER_QOS_PROFILES.xml
            Subscriber subscriber = participant.CreateSubscriber();

            // This DataReader reads data of type Temperature on Topic
            // "ChocolateTemperature". DataReader QoS is configured in
            // USER_QOS_PROFILES.xml
            DataReader<DynamicData> reader = subscriber.CreateDataReader(topic);

            var statusCondition = reader.StatusCondition;
            statusCondition.EnabledStatuses = StatusMask.DataAvailable;
            int samplesRead = 0;
            statusCondition.Triggered += _ => samplesRead += ProcessData(reader);

            var waitset = new WaitSet();
            waitset.AttachCondition(statusCondition);
            while (samplesRead < sampleCount)
            {
                Console.WriteLine("HelloWorld subscriber sleeping for 4 sec...");
                waitset.Dispatch(Duration.FromSeconds(4));
            }
        }
    }
}
