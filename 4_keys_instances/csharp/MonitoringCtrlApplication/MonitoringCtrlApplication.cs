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
using Omg.Dds.Subscription;
using Rti.Dds.Core;
using Rti.Dds.Core.Status;
using Rti.Dds.Domain;
using Rti.Dds.Publication;
using Rti.Dds.Subscription;
using Rti.Dds.Topics;

namespace KeysInstances
{
    /// <summary>
    /// MonitoringCtrlApplication application
    /// </summary>
    public class MonitoringCtrlApplication
    {
        private bool shutdownRequested;

        private void PublishStartLot(
            DataWriter<ChocolateLotState> writer,
            uint lotsToProcess)
        {
            var sample = new ChocolateLotState();
            for (uint count = 0; !shutdownRequested && count < lotsToProcess; count++)
            {
                sample.lot_id = count % 100;
                sample.lot_status = LotStatusKind.WAITING;
                sample.next_station = StationKind.TEMPERING_CONTROLLER;

                Console.WriteLine("Starting lot:");
                Console.WriteLine($"[lot_id: {sample.lot_id} next_station: {sample.next_station}]");
                writer.Write(sample);

                Thread.Sleep(8000);
            }
        }

        private int MonitorLotState(DataReader<ChocolateLotState> reader)
        {
            int samplesRead = 0;
            using var samples = reader.Take();
            foreach (var sample in samples)
            {
                Console.WriteLine("Received lot update: ");
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

        // Exercise #4.4: Add monitor_temperature function

        private void RunExample(
            int domainId = 0,
            uint lotsToProcess = 10)
        {
            // A DomainParticipant allows an application to begin communicating in
            // a DDS domain. Typically there is one DomainParticipant per application.
            // DomainParticipant QoS is configured in USER_QOS_PROFILES.xml
            DomainParticipant participant = DomainParticipantFactory.Instance
                .CreateParticipant(domainId);

            // A Topic has a name and a datatype. Create a Topic named
            // "ChocolateLotState" with type ChocolateLotState.
            Topic<ChocolateLotState> lotStateTopic = participant.CreateTopic<ChocolateLotState>(
                "ChocolateLotState");
            // Exercise #4.1: Add a Topic for Temperature to this application

            // A Publisher allows an application to create one or more DataWriters
            // Publisher QoS is configured in USER_QOS_PROFILES.xml
            Publisher publisher = participant.CreatePublisher();

            // This DataWriter writes data on Topic "ChocolateLotState"
            // DataWriter QoS is configured in USER_QOS_PROFILES.xml
            DataWriter<ChocolateLotState> lotStateWriter =
                publisher.CreateDataWriter(lotStateTopic);

            // A Subscriber allows an application to create one or more DataReaders
            // Subscriber QoS is configured in USER_QOS_PROFILES.xml
            Subscriber subscriber = participant.CreateSubscriber();

            // Create DataReader of Topic "ChocolateLotState".
            // DataReader QoS is configured in USER_QOS_PROFILES.xml
            DataReader<ChocolateLotState> lotStateReader =
                subscriber.CreateDataReader(lotStateTopic);

            // Exercise #4.2: Add a DataReader for Temperature to this application

            // Obtain the DataReader's Status Condition
            StatusCondition lotStateStatusCondition = lotStateReader.StatusCondition;

            // Enable the 'data available' status.
            lotStateStatusCondition.EnabledStatuses = StatusMask.DataAvailable;

            int lotsProcessed = 0;
            lotStateStatusCondition.Triggered +=
                _ => lotsProcessed += MonitorLotState(lotStateReader);

            // Create a WaitSet and attach the StatusCondition
            WaitSet waitset = new WaitSet();
            waitset.AttachCondition(lotStateStatusCondition);

            // Exercise #4.3: Add the new DataReader's StatusCondition to the Waitset

            var startLotTask = Task.Run(() => PublishStartLot(lotStateWriter, lotsToProcess));

            while(!shutdownRequested && lotsProcessed < lotsToProcess)
            {
                waitset.Dispatch(Duration.FromSeconds(4));
            }

            startLotTask.Wait();
        }

        /// <summary>
        /// Main function, receiving structured command-line arguments
        /// via the System.Console.DragonFruit package.
        /// For example: dotnet run -- --domain-id 54 --lots-to-process 5
        /// </summary>
        /// <param name="domainId">The domain ID to create the DomainParticipant</param>
        /// <param name="lotsToProcess">The number of data samples to publish</param>
        public static void Main(
            int domainId = 0,
            uint lotsToProcess = int.MaxValue)
        {
            var example = new MonitoringCtrlApplication();

            // Setup signal handler
            Console.CancelKeyPress += (_, eventArgs) =>
            {
                Console.WriteLine("Shuting down...");
                eventArgs.Cancel = true; // let the application shutdown gracefully
                example.shutdownRequested = true;
            };

            try
            {
                example.RunExample(domainId, lotsToProcess);
            }
            catch(Exception ex)
            {
                Console.WriteLine("RunExample exception: " + ex.Message);
                Console.WriteLine(ex.StackTrace);
            }
        }
    }
}
