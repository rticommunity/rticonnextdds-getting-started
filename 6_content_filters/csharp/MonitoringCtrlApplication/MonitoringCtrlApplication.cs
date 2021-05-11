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

namespace ContentFilter
{
    /// <summary>
    /// Example publisher application
    /// </summary>
    public class MonitoringCtrlApplication
    {
        private bool shutdownRequested;

        private void PublishStartLot(
            DataWriter<ChocolateLotState> writer,
            uint lotsToProcess)
        {
            var sample = writer.CreateData();
            for (uint count = 0; !shutdownRequested && count < lotsToProcess; count++)
            {
                sample.lot_id = count % 100;
                sample.lot_status = LotStatusKind.WAITING;
                sample.next_station = StationKind.COCOA_BUTTER_CONTROLLER;

                Console.WriteLine("Starting lot:");
                Console.WriteLine($"[lot_id: {sample.lot_id} next_station: {sample.next_station}]");
                writer.Write(sample);

                Thread.Sleep(30_000);
            }
        }

        private static int MonitorLotState(DataReader<ChocolateLotState> reader)
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
                    // Detect that a lot is complete by checking for
                    // the disposed state.
                    if (sample.Info.State.Instance == InstanceState.NotAliveDisposed)
                    {
                        // Create a sample to fill in the key values associated
                        // with the instance
                        var keyHolder = new ChocolateLotState();
                        reader.GetKeyValue(keyHolder, sample.Info.InstanceHandle);
                        Console.WriteLine($"[lot_id: {keyHolder.lot_id} is completed]");
                    }
                }
            }

            return samplesRead;
        }

        // Add MonitorTemperature function
        private static void MonitorTemperature(DataReader<Temperature> reader)
        {
            using var samples = reader.Take();
            foreach (var data in samples.ValidData())
            {
                // Receive updates from tempering station about chocolate temperature.
                // Only an error if below 30 or over 32 degrees Fahrenheit.
                Console.WriteLine("Temperature high: " + data);
            }
        }

        private void RunExample(
            int domainId = 0,
            uint lotsToProcess = 10)
        {
            // Loads the QoS from the qos_profiles.xml file.
            var qosProvider = new QosProvider("./qos_profiles.xml");

            // A DomainParticipant allows an application to begin communicating in
            // a DDS domain. Typically there is one DomainParticipant per application.
            // Load DomainParticipant QoS profile
            var participantQos = qosProvider.GetDomainParticipantQos(
                "ChocolateFactoryLibrary::MonitoringControlApplication");
            DomainParticipant participant = DomainParticipantFactory.Instance
                .CreateParticipant(domainId, participantQos);

            // A Topic has a name and a datatype. Create a Topic with type
            // ChocolateLotState.  Topic name is a constant defined in the IDL file.
            Topic<ChocolateLotState> lotStateTopic =
                participant.CreateTopic<ChocolateLotState>("ChocolateLotState");
            // Add a Topic for Temperature to this application
            Topic<Temperature> temperatureTopic =
                participant.CreateTopic<Temperature>("ChocolateTemperature");
            ContentFilteredTopic<Temperature> filteredTemperatureTopic =
                participant.CreateContentFilteredTopic(
                    name: "FilteredTemperature",
                    relatedTopic: temperatureTopic,
                    filter: new Filter(
                        expression: "degrees > %0 or degrees < %1",
                        parameters: new string[] { "32", "30" }));

            // A Publisher allows an application to create one or more DataWriters
            // Publisher QoS is configured in USER_QOS_PROFILES.xml
            Publisher publisher = participant.CreatePublisher();

            // This DataWriter writes data on Topic "ChocolateLotState"
            var writerQos = qosProvider.GetDataWriterQos(
                "ChocolateFactoryLibrary::ChocolateLotStateProfile");
            DataWriter<ChocolateLotState> lotStateWriter =
                publisher.CreateDataWriter(lotStateTopic, writerQos);

            // A Subscriber allows an application to create one or more DataReaders
            // Subscriber QoS is configured in USER_QOS_PROFILES.xml
            Subscriber subscriber = participant.CreateSubscriber();

            // Create DataReader of Topic "ChocolateLotState".
            // DataReader QoS is configured in USER_QOS_PROFILES.xml
            var readerQos = qosProvider.GetDataReaderQos(
                "ChocolateFactoryLibrary::ChocolateLotStateProfile");
            DataReader<ChocolateLotState> lotStateReader =
                subscriber.CreateDataReader(lotStateTopic, readerQos);

            // Add a DataReader for Temperature to this application
            readerQos = qosProvider.GetDataReaderQos(
                "ChocolateFactoryLibrary::ChocolateTemperatureProfile");
            DataReader<Temperature> temperatureReader =
                subscriber.CreateDataReader(filteredTemperatureTopic, readerQos);

            // Obtain the DataReader's Status Condition
            StatusCondition temperatureStatusCondition = temperatureReader.StatusCondition;
            temperatureStatusCondition.EnabledStatuses = StatusMask.DataAvailable;

            // Associate a handler with the status condition. This will run when the
            // condition is triggered, in the context of the dispatch call (see below)
            temperatureStatusCondition.Triggered +=
                _ => MonitorTemperature(temperatureReader);

            // Do the same with the lotStateReader's StatusCondition
            StatusCondition lotStateStatusCondition = lotStateReader.StatusCondition;
            lotStateStatusCondition.EnabledStatuses = StatusMask.DataAvailable;

            int lotsProcessed = 0;
            lotStateStatusCondition.Triggered +=
                _ => lotsProcessed += MonitorLotState(lotStateReader);

            // Create a WaitSet and attach the StatusCondition
            var waitset = new WaitSet();
            waitset.AttachCondition(lotStateStatusCondition);

            // Add the new DataReader's StatusCondition to the Waitset
            waitset.AttachCondition(temperatureStatusCondition);

            // Start publishing in a separate thread
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
