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
using Rti.Types.Dynamic;

namespace KeyesInstances
{
    // This Preview Release doesn't support IDL types yet. The example uses
    // XML-defined dynamically-loaded types. Topics for such types use DynamicData
    using Temperature = DynamicData;
    using ChocolateLotState = DynamicData;

    /// <summary>
    /// Example publisher application
    /// </summary>
    public class MonitoringCtrlApplication
    {
        private readonly Utils.ChocolateFactoryTypes types =
            new Utils.ChocolateFactoryTypes();
        private bool shutdownRequested = false;

        private void PublishStartLot(
            DataWriter<ChocolateLotState> writer,
            uint lotsToProcess)
        {
            var sample = writer.CreateData();
            for (uint count = 0; !shutdownRequested && count < lotsToProcess; count++)
            {
                sample.SetValue("lot_id", count % 100);
                sample.SetAnyValue("lot_status", "WAITING");
                sample.SetAnyValue("next_station", "TEMPERING_CONTROLLER");

                Console.WriteLine($"\nStarting lot:\n{sample}");
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
                Console.WriteLine("Received Lot Update: ");
                if (sample.Info.ValidData)
                {
                    Console.WriteLine(sample.Data);
                    samplesRead++;
                }
                else
                {
                    // Exercise #3.2: Detect that a lot is complete by checking for
                    // the disposed state.
                    if (sample.Info.State.Instance == InstanceState.NotAliveDisposed)
                    {
                        // Create a sample to fill in the key values associated
                        // with the instance
                        var keyHolder = new ChocolateLotState(types.ChocolateLotState);
                        reader.GetKeyValue(keyHolder, sample.Info.InstanceHandle);
                        Console.WriteLine($"[lot_id: {keyHolder.GetUInt32Value("lot_id")} is completed]");
                    }
                }
            }

            return samplesRead;
        }

        // Exercise #4.4: Add monitor_temperature function
        private void MonitorTemperature(DataReader<Temperature> reader)
        {
            using var samples = reader.Take();
            foreach (var data in samples.ValidData)
            {
                // A new exercise will show how to specify data filtering with
                // a ContentFilteredTopic.
                if (data.GetInt32Value("degrees") > 32)
                {
                    Console.WriteLine($"Temperature high: {data}");
                }
            }
        }

        private void RunExample(
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
            Topic<ChocolateLotState> lotStateTopic = participant.CreateTopic(
                "ChocolateLotState",
                types.ChocolateLotState);
            // Exercise #4.1: Add a Topic for Temperature to this application
            Topic<Temperature> temperatureTopic = participant.CreateTopic(
                "ChocolateTemperature",
                types.Temperature);

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
            DataReader<Temperature> temperatureReader =
                subscriber.CreateDataReader(temperatureTopic);

            // Obtain the DataReader's Status Condition
            StatusCondition temperatureStatusCondition = temperatureReader.StatusCondition;
            temperatureStatusCondition.EnabledStatuses = StatusMask.DataAvailable;

            // Associate a handler with the status condition. This will run when the
            // condition is triggered, in the context of the dispatch call (see below)
            temperatureStatusCondition.Triggered += _ => MonitorTemperature(temperatureReader);

            // Do the same with the lotStateReader's StatusCondition
            StatusCondition lotStateStatusCondition = lotStateReader.StatusCondition;
            lotStateStatusCondition.EnabledStatuses = StatusMask.DataAvailable;

            int lotsProcessed = 0;
            lotStateStatusCondition.Triggered +=
                _ => lotsProcessed += MonitorLotState(lotStateReader);

            // Create a WaitSet and attach the StatusCondition
            WaitSet waitset = new WaitSet();
            waitset.AttachCondition(lotStateStatusCondition);

            // Exercise #4.3: Add the new DataReader's StatusCondition to the Waitset
            waitset.AttachCondition(temperatureStatusCondition);

            var startLotTask = Task.Run(() => PublishStartLot(lotStateWriter, lotsToProcess));

            while(!shutdownRequested && lotsProcessed < lotsToProcess)
            {
                waitset.Dispatch(Duration.FromSeconds(4));
            }

            startLotTask.Wait();
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
                example.RunExample(sensorId, domainId, lotsToProcess);
            }
            catch(Exception ex)
            {
                Console.WriteLine("RunExample exception: " + ex.Message);
                Console.WriteLine(ex.StackTrace);
            }
        }
    }
}
