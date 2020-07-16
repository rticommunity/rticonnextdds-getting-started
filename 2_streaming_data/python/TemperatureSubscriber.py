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
import pathlib  # to get the location of the XML file
import random  # for generating random numbers
import argparse  # for arg parsing
import time  # for sleeping

FILE = str(pathlib.Path(__file__).parent.absolute()) + "/" + "temperature.xml"


def process_data(reader):
    # Take all samples.  Samples are loaned to application, loan is
    # returned when the variable sample goes out of scope
    samples_read = 0
    samples = reader.take()

    for sample in samples:
        if sample.info.valid:
            samples_read += 1
            print(f"Received {sample.data}")
    return samples_read


def run_example(domain_id, sample_count, sensor_id):

    # A DomainParticipant allows an application to begin communicating in
    # a DDS domain. Typically there is one DomainParticipant per application.
    # Create a DomainParticipant with default Qos
    participant = dds.DomainParticipant(domain_id)


    # A Topic has a name and a datatype. Create a Topic named
    # "ChocolateTemperature" with type Temperature
    provider_type = dds.QosProvider(FILE).type("Temperature")
    topic = dds.DynamicData.Topic(participant, "ChocolateTemperature", provider_type)

    # A Subscriber allows an application to create one or more DataReaders
    # Subscriber QoS is configured in USER_QOS_PROFILES.xml
    subscriber = dds.Subscriber(participant)

    # This DataReader reads data of type Temperature on Topic
    # "ChocolateTemperature". DataReader QoS is configured in
    # USER_QOS_PROFILES.xml
    reader = dds.DynamicData.DataReader(subscriber, topic)

    # Obtain the DataReader's Status Condition
    status_condition = dds.StatusCondition(reader)

    # Enable the 'data available' status.
    status_condition.enabled_statuses = dds.StatusMask.data_available()

    # Associate a handler with the status condition. This will run when the
    # condition is triggered, in the context of the dispatch call (see below)
    samples_read = 0

    def hander(_):
        nonlocal samples_read
        nonlocal reader
        samples_read += process_data(reader)

    status_condition.handler(hander)

    # Create a WaitSet and attach the StatusCondition
    waitset = dds.WaitSet()
    waitset += status_condition

    try:
        # Dispatch will call the handlers associated to the WaitSet conditions
        # when they activate
        while sample_count is None or samples_read < sample_count:
            print("ChocolateTemperature subcriber sleeping for 4 sec...")
            waitset.dispatch(dds.Duration(4))  # Wait up to 4s each time
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    # parse the arguments
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
