#
# (c) Copyright, Real-Time Innovations, 2020.  All rights reserved.
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
import threading
import time

import rti.connextdds as dds
from chocolate_factory import ChocolateLotState, Temperature, LotStatusKind, StationKind, CHOCOLATE_LOT_STATE_TOPIC, CHOCOLATE_TEMPERATURE_TOPIC

def publish_start_lot(lot_state_writer: dds.DataWriter, lots_to_process: int):
    sample = ChocolateLotState()
    try:
        for count in range(lots_to_process):

            # Set the values for a chocolate lot that is going to be sent to wait
            # at the tempering station
            sample.lot_id = count % 100
            sample.lot_status = LotStatusKind.WAITING
            sample.next_station = StationKind.TEMPERING_CONTROLLER

            print("\nStarting lot:")
            print(f"[lot_id: {sample.lot_id}, next_station: {str(sample.next_station)}]")

            # Send an update to station that there is a lot waiting for tempering
            lot_state_writer.write(sample)
            time.sleep(10)
    except KeyboardInterrupt:
        pass


def monitor_lot_state(reader: dds.DataReader) -> int:
    # Receive updates from stations about the state of current lots
    samples_read = 0

    for data, info in reader.take():
        print("Received lot update:")
        if info.valid:
            print(data)
            samples_read += 1
        # Detect that a lot is complete by checking for
        # the disposed state.
        elif info.state.instance_state == dds.InstanceState.NOT_ALIVE_DISPOSED:
            key_holder = reader.key_value(info.instance_handle)
            print(f"[Lot {key_holder.lot_id} is completed]")

    return samples_read


def run_example(domain_id: int, lots_to_process: int):

    # Exercise #1.1: Add QoS provider

    # A DomainParticipant allows an application to begin communicating in
    # a DDS domain. Typically there is one DomainParticipant per application.
    # Exercise #1.2: Load DomainParticipant QoS profile
    participant = dds.DomainParticipant(domain_id)

    # A Topic has a name and a datatype. Create a Topic with type
    # ChocolateLotState. Topic name is a constant defined in the IDL file.
    lot_state_topic = dds.Topic(participant, CHOCOLATE_LOT_STATE_TOPIC, ChocolateLotState)

    # Add a Topic for Temperature
    temperature_topic = dds.Topic(
        participant, CHOCOLATE_TEMPERATURE_TOPIC, Temperature)

    # A Publisher allows an application to create one or more DataWriters
    # Publisher QoS is configured in USER_QOS_PROFILES.xml
    publisher = dds.Publisher(participant)

    # This DataWriter writes data on Topic "ChocolateLotState"
    # Exercise #4.1: Load ChocolateLotState DataWriter QoS profile after
    # debugging incompatible QoS
    lot_state_writer = dds.DataWriter(publisher, lot_state_topic)

    # A Subscriber allows an application to create one or more DataReaders
    # Subscriber QoS is configured in USER_QOS_PROFILES.xml
    subscriber = dds.Subscriber(participant)

    # Create DataReader of Topic "ChocolateLotState".
    # Exercise #1.3: Update the lot_state_reader and temperature_reader
    # to use correct QoS
    lot_state_reader = dds.DataReader(subscriber, topic)

    # Add a DataReader for Temperature
    temperature_reader = dds.DataReader(subscriber, temperature_topic)

    # Obtain the DataReader's Status condition and enable the 'data available'
    # estatus.
    temperature_status_condition = dds.StatusCondition(temperature_reader)
    temperature_status_condition.enabled_statuses = dds.StatusMask.DATA_AVAILABLE

    # Define a function that processes the temperature data and prints a message
    # if it exceeds the 32 degrees.
    def monitor_temperature(_: dds.Condition):
        high_temperatures = filter(
            lambda t: t.degrees > 32, temperature_reader.take_data())
        for t in high_temperatures:
            print(f"Temperature high: {t.degrees}")

    # Associate monitor_temperature to the status condition
    temperature_status_condition.set_handler(monitor_temperature)

    # Obtain the DataReader's Status Condition
    lot_state_status_condition = dds.StatusCondition(lot_state_reader)
    # Enable the 'data available' status.
    lot_state_status_condition.enabled_statuses = dds.StatusMask.DATA_AVAILABLE

    # Associate a handler with the status condition. This will run when the
    # condition is triggered, in the context of the dispatch call (see below)
    lots_processed = 0

    def handler(_: dds.Condition):
        nonlocal lots_processed
        lots_processed += monitor_lot_state(lot_state_reader)

    lot_state_status_condition.set_handler(handler)
    # Create a WaitSet and attach the StatusConditions
    waitset = dds.WaitSet()
    waitset += lot_state_status_condition
    waitset += temperature_status_condition

    # Create a thread to periodically publish the temperature
    start_lot_thread = threading.Thread(
        target=publish_start_lot,
        args=(lot_state_writer, lots_to_process))
    start_lot_thread.start()
    try:
        while lots_processed < lots_to_process:
            # Dispatch will call the handlers associated to the WaitSet conditions
            # when they activate
            waitset.dispatch(dds.Duration(10))  # Wait for up to 10s each time
    except KeyboardInterrupt:
        pass

    start_lot_thread.join()


if __name__ == "__main__":
    # Parse the command line args
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
        "--sample-count",
        type=int,
        action="store",
        required=False,
        default=10000,
        dest="sample_count",
    )
    args = parser.parse_args()

    run_example(args.domain_id, args.sample_count)
