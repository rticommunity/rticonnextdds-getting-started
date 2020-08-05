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

import rti.connextdds as dds
import argparse  # for arg parsing
import pathlib  # for finding the XML files
import threading  # for multithreading
import time  # for sleeping
import random  # to generate random data points

FILE = str(pathlib.Path(__file__).parent.absolute()) + "/chocolate_factory.xml"

provider = dds.QosProvider(FILE)

TEMPERATURE_TYPE = provider.type("Temperature")
CHOCOLATE_LOT_TYPE = provider.type("ChocolateLotState")
STATION_KIND_TYPE = provider.type("StationKind")
LOT_STATUS_KIND_TYPE = provider.type("LotStatusKind")


CHOCOLATE_LOT_STATE_TOPIC = "ChocolateLotState"
CHOCOLATE_TEMPERATURE_TOPIC = "ChocolateTemperature"


# Tempering application:
# 1) Publishes the temperature
# 2) Subscribes to the lot state
# 3) After "processing" the lot, publishes the lot state


def publish_temperature(writer, sensor_id):

    # Create temperature sample for writing
    temperature = dds.DynamicData(TEMPERATURE_TYPE)
    try:
        while True:
            # Modify the data to be written here
            temperature["sensor_id"] = sensor_id
            temperature["degrees"] = random.randint(
                30, 32
            )  # Random value between 30 and 32

            writer.write(temperature)

            time.sleep(0.1)

    except KeyboardInterrupt:
        pass


def process_lot(lot_state_reader, lot_state_writer):
    # Take all samples.  Samples are loaned to application, loan is
    # returned when LoanedSamples destructor called.
    samples = lot_state_reader.take()

    # Process lots waiting for tempering
    for sample in samples:
        if (
            sample.info.valid
            and sample.data["next_station"]
            == STATION_KIND_TYPE["TEMPERING_CONTROLLER"].ordinal
        ):
            print(f"Processing lot #{sample.data['lot_id']}")

            # Send an update that the tempering station is processing lot
            updated_state = sample.data
            updated_state["lot_status"] = LOT_STATUS_KIND_TYPE[
                "PROCESSING"
            ].ordinal
            updated_state["next_station"] = STATION_KIND_TYPE[
                "INVALID_CONTROLLER"
            ].ordinal
            updated_state["station"] = STATION_KIND_TYPE[
                "TEMPERING_CONTROLLER"
            ].ordinal
            lot_state_writer.write(updated_state)

            # "Processing" the lot.
            time.sleep(5)

            # Exercise #3.1: Since this is the last step in processing,
            # notify the monitoring application that the lot is complete
            # using a dispose
            instance_handle = lot_state_writer.lookup_instance(updated_state)
            lot_state_writer.dispose_instance(instance_handle)

    # The LoanedSamples destructor returns the loan


def run_example(domain_id, sensor_id):
    # A DomainParticipant allows an application to begin communicating in
    # a DDS domain. Typically there is one DomainParticipant per application.
    # DomainParticipant QoS is configured in USER_QOS_PROFILES.xml
    participant = dds.DomainParticipant(domain_id)

    # A Topic has a name and a datatype. Create Topics.
    # Topic names are constants defined in the XML file.
    temperature_topic = dds.DynamicData.Topic(
        participant, CHOCOLATE_TEMPERATURE_TOPIC, TEMPERATURE_TYPE
    )
    lot_state_topic = dds.DynamicData.Topic(
        participant, CHOCOLATE_LOT_STATE_TOPIC, CHOCOLATE_LOT_TYPE
    )

    # A Publisher allows an application to create one or more DataWriters
    # Publisher QoS is configured in USER_QOS_PROFILES.xml
    publisher = dds.Publisher(participant)

    # Create DataWriters of Topics "ChocolateTemperature" & "ChocolateLotState"
    # DataWriter QoS is configured in USER_QOS_PROFILES.xml
    temperature_writer = dds.DynamicData.DataWriter(
        publisher, temperature_topic
    )
    lot_state_writer = dds.DynamicData.DataWriter(publisher, lot_state_topic)

    # A Subscriber allows an application to create one or more DataReaders
    # Subscriber QoS is configured in USER_QOS_PROFILES.xml
    subscriber = dds.Subscriber(participant)

    # Create DataReader of Topic "ChocolateLotState".
    # DataReader QoS is configured in USER_QOS_PROFILES.xml
    lot_state_reader = dds.DynamicData.DataReader(subscriber, lot_state_topic)

    # Obtain the DataReader's Status Condition
    status_condition = dds.StatusCondition(lot_state_reader)
    # Enable the 'data available' status.
    status_condition.enabled_statuses = dds.StatusMask.data_available()

    # Associate a handler with the status condition. This will run when the
    # condition is triggered, in the context of the dispatch call (see below)
    def handler(_):
        nonlocal lot_state_reader
        nonlocal lot_state_writer
        process_lot(lot_state_reader, lot_state_writer)

    # Create a WaitSet and attach the StatusCondition
    status_condition.handler(handler)

    waitset = dds.WaitSet()
    waitset += status_condition

    # Create a thread to periodically publish the temperature
    print(f"ChocolateTemperature Sensor with ID: {sensor_id} staring")
    temperature_thread = threading.Thread(
        target=publish_temperature, args=(temperature_writer, sensor_id)
    )
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
