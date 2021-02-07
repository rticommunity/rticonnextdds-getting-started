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
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using Omg.Dds.Core;
using Rti.Dds.Core;
using Rti.Dds.Core.Status;
using Rti.Dds.Domain;
using Rti.Dds.Publication;
using Rti.Dds.Subscription;
using Rti.Dds.Topics;

namespace ContentFilter
{
    /// <summary>
    /// Example subscriber application
    /// </summary>
    public class IngredientApplication
    {
        private bool shutdownRequested;

        private readonly Dictionary<StationKind, StationKind> nextStation =
            new Dictionary<StationKind, StationKind>()
            {
                { StationKind.COCOA_BUTTER_CONTROLLER, StationKind.SUGAR_CONTROLLER },
                { StationKind.SUGAR_CONTROLLER, StationKind.MILK_CONTROLLER },
                { StationKind.MILK_CONTROLLER, StationKind.VANILLA_CONTROLLER },
                { StationKind.VANILLA_CONTROLLER, StationKind.TEMPERING_CONTROLLER }
            };

        private StationKind GetNextStation(StationKind currentStation)
            => nextStation[currentStation];

        private void ProcessLot(
            StationKind currentStation,
            DataReader<ChocolateLotState> lotStateReader,
            DataWriter<ChocolateLotState> lotStateWriter)
        {
            using var samples = lotStateReader.Take();
            foreach (var sample in samples.ValidData())
            {
                // No need to check that this is the next station: content filter
                // ensures that the reader only receives lots with
                // next_station == this station
                Console.WriteLine($"Processing lot #{sample.lot_id}");

                // Send an update that the tempering station is processing lot
                var updatedState = new ChocolateLotState(sample)
                {
                    lot_status = LotStatusKind.PROCESSING,
                    next_station = StationKind.INVALID_CONTROLLER,
                    station = currentStation
                };
                lotStateWriter.Write(updatedState);

                // "Processing" the lot.
                Thread.Sleep(5000);

                // Send an update that this station is done processing lot
                updatedState.lot_status = LotStatusKind.COMPLETED;
                updatedState.station = currentStation;
                updatedState.next_station = GetNextStation(currentStation);
                lotStateWriter.Write(updatedState);
            }
        }

        private void RunExample(int domainId, string stationKind)
        {
            if (!Enum.TryParse<StationKind>(stationKind, out var currentStation))
            {
                throw new ArgumentException("Invalid station");
            }

            // A DomainParticipant allows an application to begin communicating in
            // a DDS domain. Typically there is one DomainParticipant per application.
            // Uses TemperingApplication QoS profile to set participant name.
            var qosProvider = new QosProvider("../qos_profiles.xml");

            // By specifying a default library, we can later refer to the
            // profiles without the library name
            qosProvider.DefaultLibrary = "ChocolateFactoryLibrary";

            DomainParticipant participant = DomainParticipantFactory.Instance
                .CreateParticipant(
                    domainId,
                    qosProvider.GetDomainParticipantQos("IngredientApplication"));

            Topic<Temperature> temperatureTopic =
                participant.CreateTopic<Temperature>("ChocolateTemperature");
            Topic<ChocolateLotState> lotStateTopic =
                participant.CreateTopic<ChocolateLotState>("ChocolateLotState");

            ContentFilteredTopic<ChocolateLotState> filteredLotStateTopic =
                participant.CreateContentFilteredTopic(
                    name: "FilteredLot",
                    relatedTopic: lotStateTopic,
                    filter: new Filter(
                        expression: "next_station = %0",
                        parameters: new string[] { $"'{stationKind}'" }));

            Publisher publisher = participant.CreatePublisher();

            // Create DataWriter of Topic "ChocolateLotState"
            // using ChocolateLotStateProfile QoS profile for State Data
            DataWriter<ChocolateLotState> lotStateWriter = publisher.CreateDataWriter(
                lotStateTopic,
                qosProvider.GetDataWriterQos("ChocolateLotStateProfile"));

            Subscriber subscriber = participant.CreateSubscriber();

            // Create DataReader of Topic "ChocolateLotState", filtered by
            // next_station, and using ChocolateLotStateProfile QoS profile for
            // State Data.
            DataReader<ChocolateLotState> lotStateReader = subscriber.CreateDataReader(
                filteredLotStateTopic,
                qosProvider.GetDataReaderQos("ChocolateLotStateProfile"));

            // Monitor the DataAvailable status
            StatusCondition statusCondition = lotStateReader.StatusCondition;
            statusCondition.EnabledStatuses = StatusMask.DataAvailable;
            statusCondition.Triggered +=
                _ => ProcessLot(currentStation, lotStateReader, lotStateWriter);
            var waitset = new WaitSet();
            waitset.AttachCondition(statusCondition);

            while (!shutdownRequested)
            {
                // Wait for ChocolateLotState
                Console.WriteLine("Waiting for lot");
                waitset.Dispatch(Duration.FromSeconds(10));
            }
        }

        /// <summary>
        /// Main function, receiving structured command-line arguments
        /// via the System.Console.DragonFruit package.
        /// For example: dotnet run -- --station-kind SUGAR_CONTROLLER
        /// </summary>
        /// <param name="domainId">The domain ID to create the DomainParticipant</param>
        /// <param name="stationKind">The value of StationKind to publish</param>
        public static void Main(
            int domainId = 0,
            string stationKind = "COCOA_BUTTER_CONTROLLER")
        {
            var example = new IngredientApplication();

            // Setup signal handler
            Console.CancelKeyPress += (_, eventArgs) =>
            {
                Console.WriteLine("Shuting down...");
                eventArgs.Cancel = true; // let the application shutdown gracefully
                example.shutdownRequested = true;
            };

            try
            {
                example.RunExample(domainId, stationKind);
            }
            catch (Exception ex)
            {
                Console.WriteLine("RunExample exception: " + ex.Message);
                Console.WriteLine(ex.StackTrace);
            }
        }
    }

    // TODO: codegen
    public enum StationKind
    {
        INVALID_CONTROLLER,
        COCOA_BUTTER_CONTROLLER,
        SUGAR_CONTROLLER,
        MILK_CONTROLLER,
        VANILLA_CONTROLLER,
        TEMPERING_CONTROLLER
    }

    public enum LotStatusKind
    {
        WAITING,
        PROCESSING,
        COMPLETED
    }

    public class Temperature : IEquatable<Temperature>
    {
        public Temperature()
        {
        }

        public Temperature(int degreesParam)
        {
            degrees = degreesParam;
        }

        public Temperature(Temperature other_)
        {
            degrees = other_.degrees;
        }

        public string sensor_id { get; set; }
        public int degrees { get; set; }

        public bool Equals(Temperature other)
        {
            if (other == null)
            {
                return false;
            }

            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return degrees.Equals(other.degrees);
        }
    }

    public class ChocolateLotState
    {
        public ChocolateLotState()
        {
            station = new StationKind();
            next_station = new StationKind();
            lot_status = new LotStatusKind();
        }

        public ChocolateLotState(uint lot_idParam, StationKind stationParam, StationKind next_stationParam, LotStatusKind lot_statusParam)
        {
            lot_id = lot_idParam;
            station = stationParam;
            next_station = next_stationParam;
            lot_status = lot_statusParam;
        }

        public ChocolateLotState(ChocolateLotState other_)
        {
            lot_id = other_.lot_id;
        }

        public uint lot_id { get; set; }
        public StationKind station { get; set; }
        public StationKind next_station { get; set; }
        public LotStatusKind lot_status { get; set; }
    }
}
