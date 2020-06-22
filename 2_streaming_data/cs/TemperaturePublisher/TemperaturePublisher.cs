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
using Rti.Dds.Domain;
using Rti.Dds.Publication;
using Rti.Dds.Topics;
using Rti.Types.Dynamic;

namespace StreamingData
{
    /// <summary>
    /// Example publisher application
    /// </summary>
    public static class TemperaturePublisher
    {
        /// <summary>
        /// Main function, receiving structured command-line arguments
        /// </summary>
        /// <param name="sensorId">Identifies a sensor</param>
        /// <param name="domainId">The domain ID to create the DomainParticipant</param>
        /// <param name="sampleCount">The number of data samples to publish</param>
        public static void Main(
            string sensorId = "default_id",
            int domainId = 0,
            int sampleCount = 10)
        {
            // A DomainParticipant allows an application to begin communicating in
            // a DDS domain. Typically there is one DomainParticipant per application.
            // DomainParticipant QoS is configured in USER_QOS_PROFILES.xml
            DomainParticipant participant = DomainParticipantFactory.Instance
                .CreateParticipant(domainId);

            // A Topic has a name and a datatype. Create a Topic named
            // "ChocolateTemperature" with type Temperature
            // In this example we use a DynamicType defined in XML, which creates
            // a DynamicData topic.
            Topic<DynamicData> topic = participant.CreateTopic(
                "ChocolateTemperature",
                Utils.GetTemperatureType());

            // A Publisher allows an application to create one or more DataWriters
            // Publisher QoS is configured in USER_QOS_PROFILES.xml
            Publisher publisher = participant.CreatePublisher();

            // This DataWriter writes data on Topic "ChocolateTemperature"
            // DataWriter QoS is configured in USER_QOS_PROFILES.xml
            DataWriter<DynamicData> writer = publisher.CreateDataWriter(topic);

            // Create a DynamicData sample for writing
            var sample = writer.CreateData();
            Random rand = new Random();
            for (int count = 0; count < sampleCount; count++)
            {
                // Modify the data to be written here
                sample.SetValue("sensor_id", sensorId);
                sample.SetValue("degrees", rand.Next(30, 33));

                Console.WriteLine($"Writing ChocolateTemperature, count {count}");
                writer.Write(sample);

                // Exercise: Change this to sleep 100 ms in between writing temperatures
                Thread.Sleep(4000);
            }
        }
    }
}
