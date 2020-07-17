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
import pathlib  # for getting the location of the XML file
import threading  # For multithreading
import time  # For sleeping

FILE = str(pathlib.Path(__file__).parent.absolute()) + "/chocolate_factory.xml"

provider = dds.QosProvider(FILE)
CHOCOLATE_LOT_TYPE = provider.type("ChocolateLotState")
LOT_STATUS_KIND_TYPE = provider.type("LotStatusKind")
STATION_KIND_TYPE = provider.type("StationKind")


def publish_start_lot(writer, lots_to_process):
    sample = dds.DynamicData(CHOCOLATE_LOT_TYPE)
    try:
        count = 0
        while lots_to_process is None or count < lots_to_process:

            # Set the values for a chocolate lot that is going to be sent to wait
            # at the tempering station
            sample["lot_id"] = count % 100
            # Enumeration members expect an integer,
            # which can be obtained from `LOT_STATUS_KIND_TYPE[name].ordinal
            sample["lot_status"] = LOT_STATUS_KIND_TYPE["WAITING"].ordinal
            sample["next_station"] = STATION_KIND_TYPE["TEMPERING_CONTROLLER"].ordinal
            print(
                f"Starting lot:\n[lot_id: {sample['lot_id']} next_station: {sample['next_station']}]"
            )

            # Send an update to station that there is a lot waiting for tempering
            writer.write(sample)
            time.sleep(8)
            count += 1
    except KeyboardInterrupt:
        pass


def monitor_lot_state(reader):
    # Take all samples.  Samples are loaned to application, loan is
    # returned when LoanedSamples destructor called.
    samples_read = 0
    samples = reader.take()

    # Receive updates from stations about the state of current lots
    for sample in samples:
        if sample.info.valid:
            print("Recieved lot update: " + str(sample.data))
            samples_read += 1
        else:
            # Exercise #3.2: Detect that a lot is complete by checking for
            # the disposed state.
            if sample.info.state.instance_state == dds.InstanceState.not_alive_disposed():
                key_holder = reader.key_value(sample.info.instance_handle)
                print(f"[lot_id: {key_holder['lot_id']} is completed]")

    return samples_read


# Exercise #4.4: Add monitor_temperature function
def monitor_temperature(reader):
    samples = reader.take()
    for sample in samples:
        if sample.info.valid and sample["degrees"] > 32:
            print(f"Temperature high: {sample}")


def run_example(domain_id, lots_to_process, sensor_id):

    # A DomainParticipant allows an application to begin communicating in
    # a DDS domain. Typically there is one DomainParticipant per application.
    # DomainParticipant QoS is configured in USER_QOS_PROFILES.xml
    participant = dds.DomainParticipant(domain_id)

    # A Topic has a name and a datatype. Create a Topic with type
    # ChocolateLotState.  Topic name is a constant defined in the XML file.
    topic = dds.DynamicData.Topic(participant, "ChocolateLotState", CHOCOLATE_LOT_TYPE)

    # Exercise #4.1: Add a Topic for Temperature to this application
    temperature_topic = dds.DynamicData.Topic(
        participant, "ChocolateTemperature", provider.type("Temperature")
    )

    # A Publisher allows an application to create one or more DataWriters
    # Publisher QoS is configured in USER_QOS_PROFILES.xml
    publisher = dds.Publisher(participant)

    # This DataWriter writes data on Topic "ChocolateLotState"
    # DataWriter QoS is configured in USER_QOS_PROFILES.xml
    writer = dds.DynamicData.DataWriter(publisher, topic)

    # A Subscriber allows an application to create one or more DataReaders
    # Subscriber QoS is configured in USER_QOS_PROFILES.xml
    subscriber = dds.Subscriber(participant)

    # Create DataReader of Topic "ChocolateLotState".
    # DataReader QoS is configured in USER_QOS_PROFILES.xml
    reader = dds.DynamicData.DataReader(subscriber, topic)

    # Exercise #4.2: Add a DataReader for Temperature to this application
    temperature_reader = dds.DynamicData.DataReader(subscriber, temperature_topic)

    # Obtain the DataReader's Status Condition
    status_condition = dds.StatusCondition(reader)
    # Enable the 'data available' status.
    status_condition.enabled_statuses = dds.StatusMask.data_available()

    # Associate a handler with the status condition. This will run when the
    # condition is triggered, in the context of the dispatch call (see below)
    lots_processed = 0

    def handler(_):
        nonlocal lots_processed
        nonlocal reader
        lots_processed += monitor_lot_state(reader)

    status_condition.handler(handler)

    temperature_status_condition = dds.StatusCondition(reader)

    temperature_status_condition.enabled_statuses = dds.StatusMask.data_available()

    def temp_handler(_):
        nonlocal temperature_reader
        monitor_lot_state(temperature_reader)

    temperature_status_condition.handler(temp_handler)

    # Create a WaitSet and attach the StatusCondition
    waitset = dds.WaitSet()
    waitset += status_condition

    # Exercise #4.3: Add the new DataReader's StatusCondition to the Waitset
    waitset += temperature_status_condition

    # Create a thread to periodically publish the temperature
    start_lot_thread = threading.Thread(
        target=publish_start_lot, args=(writer, lots_to_process)
    )
    start_lot_thread.start()
    try:
        while lots_to_process is None or lots_processed < lots_to_process:
            # Dispatch will call the handlers associated to the WaitSet conditions
            # when they activate
            waitset.dispatch(dds.Duration(4))  # Wait for up to 4s each time
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
        default=None,
        dest="sample_count",
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

    run_example(args.domain_id, args.sample_count, args.sensor_id)
