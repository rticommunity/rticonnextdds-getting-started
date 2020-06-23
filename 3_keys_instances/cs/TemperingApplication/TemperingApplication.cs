﻿/*
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
using System.Threading.Tasks;
using Omg.Dds.Core;
using Rti.Dds.Core;
using Rti.Dds.Core.Status;
using Rti.Dds.Domain;
using Rti.Dds.Publication;
using Rti.Dds.Subscription;
using Rti.Dds.Topics;
using Rti.Types.Dynamic;

namespace KeyesInstances
{
    // This Preview Release doesn't support IDL types yet. The example uses
    // XML-defined dynamically-loaded types. Topics for such types use DynamicData
    using Temperature = DynamicData;
    using ChocolateLotState = DynamicData;

    /// <summary>
    /// Example subscriber application
    /// </summary>
    public static class TemperatureSubscriber
    {
        private static readonly Random rand = new Random();

        private static void PublishTemperature(
            DataWriter<Temperature> writer,
            string sensorId)
        {
            // Create temperature sample for writing
            var temperature = writer.CreateData();
            while (true)
            {
                // Modify the data to be written here
                temperature.SetValue("sensor_id", sensorId);
                temperature.SetValue("degrees", rand.Next(30, 33));

                writer.Write(temperature);

                Thread.Sleep(100);
            }
        }

        private static void ProcessLot(
            DataReader<ChocolateLotState> lotStateReader,
            DataWriter<ChocolateLotState> lotStateWriter)
        {
            using var samples = lotStateReader.Take();
            foreach (var sample in samples.ValidData)
            {
                if (sample.GetInt32Value("next_station") == 6) // TODO: use enumerator
                {
                    uint lotId = sample.GetUInt32Value("lot_id");
                    Console.WriteLine($"Processing lot #{lotId}");

                    // Send an update that the tempering station is processing lot
                    var updatedState = lotStateWriter.CreateData();
                    updatedState.SetAnyValue("lot_status", "PROCESSING");
                    updatedState.SetAnyValue("next_station", "INVALID_CONTROLLER");
                    updatedState.SetAnyValue("station", "TEMPERING_CONTROLLER");
                    lotStateWriter.Write(updatedState);

                    // "Processing" the lot.
                    Thread.Sleep(5000);

                    // Exercise #3.1: Since this is the last step in processing,
                    // notify the monitoring application that the lot is complete
                    // using a dispose
                }
            }
        }

        /// <summary>
        /// Main function, receiving structured command-line arguments
        /// </summary>
        /// <param name="domainId">The domain ID to create the DomainParticipant</param>
        /// <param name="sensorId">Identifies the sensor ID used in the example</param>
        public static void Main(int domainId = 0, string sensorId = "default_id")
        {
            // A DomainParticipant allows an application to begin communicating in
            // a DDS domain. Typically there is one DomainParticipant per application.
            // DomainParticipant QoS is configured in USER_QOS_PROFILES.xml
            DomainParticipant participant = DomainParticipantFactory.Instance
                .CreateParticipant(domainId);

            // A Topic has a name and a datatype. Create Topics using the types
            // defined in chocolate_factory.xml
            var provider = new QosProvider("../chocolate_factory.xml");
            Topic<Temperature> temperatureTopic = participant.CreateTopic(
                "ChocolateTemperature",
                provider.GetType("Temperature"));
            Topic<ChocolateLotState> lotStateTopic = participant.CreateTopic(
                "ChocolateLotState",
                provider.GetType("ChocolateLotState"));

            // A Publisher allows an application to create one or more DataWriters
            // Publisher QoS is configured in USER_QOS_PROFILES.xml
            Publisher publisher = participant.CreatePublisher();

            // Create DataWriters of Topics "ChocolateTemperature" & "ChocolateLotState"
            // DataWriter QoS is configured in USER_QOS_PROFILES.xml
            DataWriter<Temperature> temperatureWriter =
                publisher.CreateDataWriter(temperatureTopic);
            DataWriter<ChocolateLotState> lotStateWriter =
                publisher.CreateDataWriter(lotStateTopic);

            // A Subscriber allows an application to create one or more DataReaders
            // Subscriber QoS is configured in USER_QOS_PROFILES.xml
            Subscriber subscriber = participant.CreateSubscriber();

            // This DataReader reads data of type Temperature on Topic
            // "ChocolateTemperature". DataReader QoS is configured in
            // USER_QOS_PROFILES.xml
            DataReader<ChocolateLotState> lotStateReader =
                subscriber.CreateDataReader(lotStateTopic);

            // Obtain the DataReader's Status Condition
            StatusCondition statusCondition = lotStateReader.StatusCondition;

            // Enable the 'data available' status.
            statusCondition.EnabledStatuses = StatusMask.DataAvailable;

            // Associate an event handler with the status condition.
            // This will run when the condition is triggered, in the context of
            // the dispatch call (see below)
            statusCondition.Triggered +=
                _ => ProcessLot(lotStateReader, lotStateWriter);

            // Create a WaitSet and attach the StatusCondition
            var waitset = new WaitSet();
            waitset.AttachCondition(statusCondition);

            // Create a thread to periodically publish the temperature
            Console.WriteLine($"ChocolateTemperature Sensor with ID: {sensorId} starting");
            var temperatureTask = Task.Run(
                () => PublishTemperature(temperatureWriter, sensorId));

            bool shutdownRequested = false;
            Console.CancelKeyPress += (_, _1) => shutdownRequested = true;
            while (!shutdownRequested)
            {
                // Dispatch will call the handlers associated to the WaitSet
                // conditions when they activate
                Console.WriteLine("ChocolateTemperature subscriber sleeping for 4 sec...");
                waitset.Dispatch(Duration.FromSeconds(4));
            }

            temperatureTask.Wait();
        }
    }
}