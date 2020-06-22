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
    /// <summary>
    /// Example publisher application
    /// </summary>
    public static class MonitoringCtrlApplication
    {
        private static void PublishStartLot(DataWriter<DynamicData> writer, uint lotsToProcess)
        {
            var sample = writer.CreateData();
            for (uint count = 0; count < lotsToProcess; count++)
            {
                sample.SetValue("lot_id", count % 100);
                sample.SetAnyValue("lot_status", "WAITING");
                sample.SetAnyValue("next_station", "TEMPERING_CONTROLLER");
                Console.WriteLine("\nStarting lot:");
                Console.WriteLine($"[lot_id: {sample.GetAnyValue("lot_id")} next_station: {sample.GetAnyValue("next_station")}]");
                writer.Write(sample);

                Thread.Sleep(8000);
            }
        }

        private static int MonitorLotState(DataReader<DynamicData> reader)
        {
            int samplesRead = 0;
            using var samples = reader.Take();
            foreach (var sample in samples)
            {
                if (sample.Info.ValidData)
                {
                    Console.WriteLine(sample.Data);
                    samplesRead++;
                }
                else
                {
                    // Exercise #3.2: Detect that a lot is complete by checking for
                    // the disposed state.

                }
            }

            return samplesRead;
        }

        /// <summary>
        /// Main function, receiving structured command-line arguments
        /// </summary>
        /// <param name="sensorId">Identifies a sensor</param>
        /// <param name="domainId">The domain ID to create the DomainParticipant</param>
        /// <param name="lotsToProcess">The number of data samples to publish</param>
        public static void Main(
            string sensorId = "default_id",
            int domainId = 0,
            uint lotsToProcess = 10)
        {
            // A DomainParticipant allows an application to begin communicating in
            // a DDS domain. Typically there is one DomainParticipant per application.
            // DomainParticipant QoS is configured in USER_QOS_PROFILES.xml
            DomainParticipant participant = DomainParticipantFactory.Instance
                .CreateParticipant(domainId);

            // A Topic has a name and a datatype. Create a Topic named
            // "ChocolateLotState" with type ChocolateLotState
            // In this example we use a DynamicType defined in XML, which creates
            // a DynamicData topic.
            Topic<DynamicData> topic = participant.CreateTopic(
                Utils.ChocolateLotStateTopicName,
                Utils.GetChocolateLotStateType());
            // Exercise #4.1: Add a Topic for Temperature to this application

            // A Publisher allows an application to create one or more DataWriters
            // Publisher QoS is configured in USER_QOS_PROFILES.xml
            Publisher publisher = participant.CreatePublisher();

            // This DataWriter writes data on Topic "ChocolateLotState"
            // DataWriter QoS is configured in USER_QOS_PROFILES.xml
            DataWriter<DynamicData> writer = publisher.CreateDataWriter(topic);

            // A Subscriber allows an application to create one or more DataReaders
            // Subscriber QoS is configured in USER_QOS_PROFILES.xml
            Subscriber subscriber = participant.CreateSubscriber();

            // Create DataReader of Topic "ChocolateLotState".
            // DataReader QoS is configured in USER_QOS_PROFILES.xml
            DataReader<DynamicData> reader = subscriber.CreateDataReader(topic);
            // Exercise #4.2: Add a DataReader for Temperature to this application

            // Obtain the DataReader's Status Condition
            StatusCondition statusCondition = reader.StatusCondition;
            statusCondition.EnabledStatuses = StatusMask.DataAvailable;

            // Associate a handler with the status condition. This will run when the
            // condition is triggered, in the context of the dispatch call (see below)
            int lotsProcessed = 0;
            statusCondition.Triggered +=
                _ => lotsProcessed += MonitorLotState(reader);

            // Create a WaitSet and attach the StatusCondition
            WaitSet waitset = new WaitSet();
            waitset.AttachCondition(statusCondition);

            var startLotTask = Task.Run(() => PublishStartLot(writer, lotsToProcess));

            while(lotsProcessed < lotsToProcess)
            {
                waitset.Dispatch(Duration.FromSeconds(10));
            }

            startLotTask.Wait();
        }
    }
}
