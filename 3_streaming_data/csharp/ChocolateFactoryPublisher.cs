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

namespace StreamingData
{
    /// <summary>
    /// Example publisher application
    /// </summary>
    public class ChocolateFactoryPublisher : IChocolateFactoryApplication
    {
        private bool shutdownRequested;
        private string sensorId;

        public ChocolateFactoryPublisher(string sensorId)
        {
            this.sensorId = sensorId;
        }

        public void Run(int domainId, int sampleCount)
        {
            // A DomainParticipant allows an application to begin communicating in
            // a DDS domain. Typically there is one DomainParticipant per application.
            // DomainParticipant QoS is configured in USER_QOS_PROFILES.xml
            DomainParticipant participant = DomainParticipantFactory.Instance
                .CreateParticipant(domainId);

            // A Topic has a name and a datatype. Create a Topic named
            // "ChocolateTemperature" with type Temperature
            Topic<Temperature> topic = participant.CreateTopic<Temperature>("ChocolateTemperature");

            // Exercise #2.1: Add new Topic

            // A Publisher allows an application to create one or more DataWriters
            // Publisher QoS is configured in USER_QOS_PROFILES.xml
            Publisher publisher = participant.CreatePublisher();

            // This DataWriter writes data on Topic "ChocolateTemperature"
            // DataWriter QoS is configured in USER_QOS_PROFILES.xml
            DataWriter<Temperature> writer = publisher.CreateDataWriter(topic);

            // Create a DynamicData sample for writing
            var sample = new Temperature();

            // Exercise #2.2: Add new DataWriter and data sample

            var rand = new Random();
            for (int count = 0; count < sampleCount && !shutdownRequested; count++)
            {
                // Modify the data to be written here
                sample.sensor_id = sensorId;
                sample.degrees = rand.Next(30, 33);  // Random number between 30 and 32

                Console.WriteLine($"Writing ChocolateTemperature, count {count}");
                writer.Write(sample);

                // Exercise #2.3 Write data with new ChocolateLotState DataWriter

                // Exercise #1.1: Change this to sleep 100 ms in between writing temperatures
                Thread.Sleep(4000);
            }
        }

        public void Stop() => shutdownRequested = true;
    }
}
