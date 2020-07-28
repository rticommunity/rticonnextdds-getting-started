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
using System.Threading;
using Rti.Dds.Core;
using Rti.Dds.Domain;
using Rti.Dds.Publication;
using Rti.Dds.Topics;
using Rti.Types.Dynamic;

namespace HelloWorld
{
    /// <summary>
    /// Example publisher application
    /// </summary>
    public static class HelloWorldPublisher
    {
        /// <summary>
        /// Main function, receiving structured command-line arguments, for example:
        /// dotnet run -- --domain-id 54 --sample-count 5
        /// </summary>
        /// <param name="domainId">The domain ID to create the DomainParticipant</param>
        /// <param name="sampleCount">The number of data samples to publish</param>
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
            // "HelloWorld" in hello_world.xml. To get the type we use a QosProvider
            var provider = new QosProvider("../hello_world.xml");
            Topic<DynamicData> topic = participant.CreateTopic(
                "Example HelloWorld",
                provider.GetType("HelloWorld"));

            // A Publisher allows an application to create one or more DataWriters
            // Publisher QoS is configured in USER_QOS_PROFILES.xml
            Publisher publisher = participant.CreatePublisher();

            // This DataWriter will write data on Topic "HelloWorld Topic"
            // DataWriter QoS is configured in USER_QOS_PROFILES.xml
            DataWriter<DynamicData> writer = publisher.CreateDataWriter(topic);

            var sample = writer.CreateData();
            for (int count = 0; count < sampleCount; count++)
            {
                // Modify the data to be written here
                sample.SetValue("msg", $"Hello {count}");

                Console.WriteLine($"Writing {sample}");
                writer.Write(sample);

                Thread.Sleep(1000);
            }
        }
    }
}
