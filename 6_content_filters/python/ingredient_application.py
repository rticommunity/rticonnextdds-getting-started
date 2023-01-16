#
# (c) Copyright, Real-Time Innovations, 2022.  All rights reserved.
# RTI grants Licensee a license to use, modify, compile, and create derivative
# works of the software solely for use with RTI Connext DDS. Licensee may
# redistribute copies of the software provided that all such copies are subject
# to this license. The software is provided "as is", with no warranty of any
# type, including any warranty for fitness for any purpose. RTI is under no
# obligation to maintain or support the software. RTI shall not be liable for
# any incidental or consequential damages arising out of the use or inability
# to use the software.
#

import argparse
import time

import rti.connextdds as dds
from chocolate_factory import ChocolateLotState, LotStatusKind, StationKind, CHOCOLATE_LOT_STATE_TOPIC

# Ingredient application:
# 1) Subscribes to the lot state
# 2) "Processes" the lot. (In this example, that means sleep for a time)
# 3) After "processing" the lot, publishes an updated lot state


NEXT_STATION = {
    StationKind.COCOA_BUTTER_CONTROLLER: StationKind.SUGAR_CONTROLLER,
    StationKind.SUGAR_CONTROLLER: StationKind.MILK_CONTROLLER,
    StationKind.MILK_CONTROLLER: StationKind.VANILLA_CONTROLLER,
    StationKind.VANILLA_CONTROLLER: StationKind.TEMPERING_CONTROLLER
}

def process_lot(
    station_kind: StationKind,
    lot_state_reader: dds.DataReader,
    lot_state_writer: dds.DataWriter
):
    # Process lots waiting for ingredients
    for data in lot_state_reader.take_data():
        # No need to check that this is the next station: content filter
        # ensures that the reader only receives lots with
        # next_station == this station
        print(f"Processing lot #{data.lot_id}")

        # Send an update that this station is processing lot
        updated_state = ChocolateLotState(
            lot_id=data.lot_id,
            lot_status=LotStatusKind.PROCESSING,
            station=station_kind,
            next_station=StationKind.INVALID_CONTROLLER)
        lot_state_writer.write(updated_state)

        # "Processing" the lot.
        time.sleep(5)

        # Send an update that this station is done processing lot
        updated_state.lot_status = LotStatusKind.COMPLETED
        updated_state.next_station = NEXT_STATION[station_kind]
        lot_state_writer.write(updated_state)


def run_example(domain_id: int, station_name: str):
    print(f"{station_name} station starting")

    current_station = StationKind[station_name]

    # Loads the QoS from the qos_profile.xml file
    qos_provider = dds.QosProvider("./qos_profiles.xml")

    # A DomainParticipant allows an application to begin communicating in
    # a DDS domain. Typically there is one DomainParticipant per application.
    # Uses IngredientApplication QoS profile to set participant name.
    participant = dds.DomainParticipant(
        domain_id,
        qos=qos_provider.participant_qos_from_profile(
            "ChocolateFactoryLibrary::IngredientApplication"))

    # A Topic has a name and a datatype. Create Topics.
    # Topic names are constants defined in the IDL file.
    lot_state_topic = dds.Topic(
        participant,
        CHOCOLATE_LOT_STATE_TOPIC,
        ChocolateLotState)

    filtered_lot_state_topic = dds.ContentFilteredTopic(
        lot_state_topic,
        "FilteredLot",
        dds.Filter("next_station = %0", [f"'{station_name}'"]))

    # A Publisher allows an application to create one or more DataWriters
    # Create Publisher with default QoS
    publisher = dds.Publisher(participant)

    # Create DataWriter of Topic "ChocolateLotState"
    # using ChocolateLotStateProfile QoS profile for State Data
    lot_state_writer = dds.DataWriter(
        publisher,
        lot_state_topic,
        qos=qos_provider.datawriter_qos_from_profile(
            "ChocolateFactoryLibrary::ChocolateLotStateProfile"))

    # A Subscriber allows an application to create one or more DataReaders
    subscriber = dds.Subscriber(participant)

    # Create DataReader of Topic "ChocolateLotState".
    # using ChocolateLotStateProfile QoS profile for State Data
    lot_state_reader = dds.DataReader(
        subscriber,
        filtered_lot_state_topic,
        qos=qos_provider.datareader_qos_from_profile(
            "ChocolateFactoryLibrary::ChocolateLotStateProfile"))

    # Obtain the DataReader's Status Condition and enable the the
    # 'data available' status
    status_condition = dds.StatusCondition(lot_state_reader)
    status_condition.enabled_statuses = dds.StatusMask.DATA_AVAILABLE

    # Associate a handler with the status condition. This will run when the
    # condition is triggered, in the context of the dispatch call (see below)
    def handler(_: dds.Condition):
        process_lot(current_station, lot_state_reader, lot_state_writer)
    status_condition.set_handler(handler)

    # Create a WaitSet and attach the StatusCondition
    waitset = dds.WaitSet()
    waitset += status_condition

    try:
        while True:
            # Wait for ChocolateLotState
            print("Waiting for lot")
            waitset.dispatch(dds.Duration(10))  # Wait up to 10s for update
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    # Parse the args
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--domain-id",
        type=int,
        action="store",
        required=False,
        default=0,
        dest="domain_id",
    )
    parser.add_argument(
        "--station-kind",
        type=str,
        action="store",
        required=True,
        dest="station_kind",
    )
    args = parser.parse_args()

    run_example(args.domain_id, args.station_kind)
