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
import rti.connextdds as dds
from chocolate_factory import Temperature

def run_example(domain_id, sample_count):

    # A DomainParticipant allows an application to begin communicating in
    # a DDS domain. Typically there is one DomainParticipant per application.
    # Create a DomainParticipant with default Qos
    participant = dds.DomainParticipant(domain_id)

    # A Topic has a name and a datatype. Create a Topic named
    # "ChocolateTemperature" with type Temperature
    topic = dds.Topic(participant, "ChocolateTemperature", Temperature)

    # A Subscriber allows an application to create one or more DataReaders
    # Subscriber QoS is configured in USER_QOS_PROFILES.xml
    subscriber = dds.Subscriber(participant)

    # This DataReader reads data of type Temperature on Topic
    # "ChocolateTemperature". DataReader QoS is configured in
    # USER_QOS_PROFILES.xml
    reader = dds.DataReader(subscriber, topic)

    # Obtain the DataReader's Status Condition
    status_condition = dds.StatusCondition(reader)

    # Enable the 'data available' status.
    status_condition.enabled_statuses = dds.StatusMask.DATA_AVAILABLE

    # Associate a handler with the status condition. This will run when the
    # condition is triggered, in the context of the dispatch call (see below)
    samples_read = 0

    def process_data(_):
        nonlocal samples_read
        nonlocal reader
        samples = reader.take_data()
        for sample in samples:
            print(sample)

        samples_read += len(samples)

    status_condition.set_handler(process_data)

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

    args = parser.parse_args()

    run_example(args.domain_id, args.sample_count)
