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
import threading
import time
import random

import rti.connextdds as dds
from chocolate_factory import ChocolateLotState, Temperature, LotStatusKind, StationKind, CHOCOLATE_LOT_STATE_TOPIC, CHOCOLATE_TEMPERATURE_TOPIC

# Tempering application:
# 1) Publishes the temperature
# 2) Subscribes to the lot state
# 3) After "processing" the lot, publishes the lot state

def publish_temperature(writer, sensor_id):

    # Create temperature sample for writing
    temperature = Temperature()
    try:
        while True:
            # Modify the data to be written here
            temperature.sensor_id = sensor_id
            temperature.degrees = random.randint(30, 32)

            writer.write(temperature)

            time.sleep(0.1)

    except KeyboardInterrupt:
        pass


def process_lot(
    lot_state_reader: dds.DataReader,
    lot_state_writer: dds.DataWriter
):
    # Process lots waiting for tempering
    for data in lot_state_reader.take_data():
        # Exercise #1.3: Remove the check that the Tempering Application is
        # the next_station. This will now be filtered automatically.
        if data.next_station == StationKind.TEMPERING_CONTROLLER:
            print(f"Processing lot #{data.lot_id}")

            # Send an update that the tempering station is processing lot
            updated_state = ChocolateLotState(
                lot_id=data.lot_id,
                lot_status=LotStatusKind.PROCESSING,
                station=StationKind.TEMPERING_CONTROLLER,
                next_station=StationKind.INVALID_CONTROLLER)
            lot_state_writer.write(updated_state)

            # "Processing" the lot.
            time.sleep(5)

            # Since this is the last step in processing,
            # notify the monitoring application that the lot is complete
            # using a dispose
            lot_state_writer.dispose_instance(
              lot_state_writer.lookup_instance(updated_state))
            print("Lot completed")


def on_requested_incompatible_qos(reader: dds.DataReader):
    incompatible_policy = reader.requested_incompatible_qos_status.last_policy_id
    print(f"Discovered DataWriter with incompatible policy: {incompatible_policy}")


def run_example(domain_id, sensor_id):
    # Loads the QoS from the qos_profile.xml file
    qos_provider = dds.QosProvider("./qos_profiles.xml")


    # A DomainParticipant allows an application to begin communicating in
    # a DDS domain. Typically there is one DomainParticipant per application.
    # Uses TemperingApplication QoS profile to set participant name.
    participant = dds.DomainParticipant(
        domain_id,
        qos=qos_provider.participant_qos_from_profile(
            "ChocolateFactoryLibrary::TemperingApplication"))

    # A Topic has a name and a datatype. Create Topics.
    # Topic names are constants defined in the IDL file.
    temperature_topic = dds.Topic(
        participant,
        CHOCOLATE_TEMPERATURE_TOPIC,
        Temperature)
    lot_state_topic = dds.Topic(
        participant,
        CHOCOLATE_LOT_STATE_TOPIC,
        ChocolateLotState)

    # Exercise #1.1: Create a Content-Filtered Topic that filters out
    # chocolate lot state unless the next_station = TEMPERING_CONTROLLER

    # A Publisher allows an application to create one or more DataWriters
    # Publisher QoS is configured in USER_QOS_PROFILES.xml
    publisher = dds.Publisher(participant)

    # Create DataWriters of Topic "ChocolateTemperature"
    # using ChocolateTemperatureProfile QoS profile for Streaming Data
    temperature_writer = dds.DataWriter(
        publisher,
        temperature_topic,
        qos=qos_provider.datawriter_qos_from_profile(
            "ChocolateFactoryLibrary::ChocolateTemperatureProfile"))

    # Create DataWriter of Topic "ChocolateLotState"
    # using ChocolateLotStateProfile QoS profile for State Data
    lot_state_writer = dds.DataWriter(
        publisher,
        lot_state_topic,
        qos=qos_provider.datawriter_qos_from_profile(
            "ChocolateFactoryLibrary::ChocolateLotStateProfile"))

    # A Subscriber allows an application to create one or more DataReaders
    # Subscriber QoS is configured in USER_QOS_PROFILES.xml
    subscriber = dds.Subscriber(participant)

    # Create DataReader of Topic "ChocolateLotState".
    # using ChocolateLotStateProfile QoS profile for State Data
    # Exercise #1.2: Change the DataReader's Topic to use a
    # Content-Filtered Topic
    lot_state_reader = dds.DataReader(
        subscriber,
        lot_state_topic,
        qos=qos_provider.datareader_qos_from_profile(
            "ChocolateFactoryLibrary::ChocolateLotStateProfile"))

    # Obtain the DataReader's Status Condition and enable the the 
    # 'data available' and 'requested incompatible qos' statuses
    status_condition = dds.StatusCondition(lot_state_reader)
    status_condition.enabled_statuses = dds.StatusMask.DATA_AVAILABLE | dds.StatusMask.REQUESTED_INCOMPATIBLE_QOS

    # Associate a handler with the status condition. This will run when the
    # condition is triggered, in the context of the dispatch call (see below)
    def handler(_: dds.Condition):
        status = lot_state_reader.status_changes
        if dds.StatusMask.DATA_AVAILABLE in status:
            process_lot(lot_state_reader, lot_state_writer)

        if dds.StatusMask.REQUESTED_INCOMPATIBLE_QOS in status:
            on_requested_incompatible_qos(lot_state_reader)

    status_condition.set_handler(handler)

    # Create a WaitSet and attach the StatusCondition
    waitset = dds.WaitSet()
    waitset += status_condition

    # Create a thread to periodically publish the temperature
    print(f"ChocolateTemperature Sensor with ID: {sensor_id} starting")
    temperature_thread = threading.Thread(
        target=publish_temperature,
        args=(temperature_writer, sensor_id))
    temperature_thread.start()
    try:
        while True:
            # Wait for ChocolateLotState
            print("Waiting for lot")
            waitset.dispatch(dds.Duration(10))  # Wait up to 10s for update
    except KeyboardInterrupt:
        pass

    temperature_thread.join()


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
        "--sensor-id",
        type=str,
        action="store",
        required=False,
        default="0",
        dest="sensor_id",
    )
    args = parser.parse_args()

    run_example(args.domain_id, args.sensor_id)
