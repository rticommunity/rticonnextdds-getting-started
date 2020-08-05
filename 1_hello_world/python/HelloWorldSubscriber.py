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
import time  # for sleep
import pathlib  # for finding the xml file

FILE = str(pathlib.Path(__file__).parent.absolute()) + "/" + "HelloWorld.xml"


def process_data(reader):

    # Take all samples.  Samples are loaned to application, loan is
    # returned when LoanedSamples destructor called or when you return
    # the loan explicitly.
    samples_read = 0
    samples = reader.take()

    for sample in samples:
        if sample.info.valid:
            print(f"Received: {sample.data}")
            samples_read += 1

    return samples_read


def run_example(domain_id, sample_count):

    # A DomainParticipant allows an application to begin communicating in
    # a DDS domain. Typically there is one DomainParticipant per application.
    # Create a DomainParticipant with default Qos
    with dds.DomainParticipant(domain_id) as participant:
        # A QosProvider is used here to get the type from the file
        # HelloWorld.xml
        provider = dds.QosProvider(FILE)

        provider_type = provider.type("HelloWorld")

        # A Topic has a name and a datatype. Create a Topic named
        # "HelloWorld Topic" with type HelloWorld
        topic = dds.DynamicData.Topic(
            participant, "Example HelloWorld", provider_type
        )

        # A Subscriber allows an application to create one or more DataReaders
        # Subscriber QoS is configured in USER_QOS_PROFILES.xml
        subscriber = dds.Subscriber(participant)

        # This DataReader will read data of type HelloWorld on Topic
        # "HelloWorld Topic". DataReader QoS is configured in
        # USER_QOS_PROFILES.xml
        reader = dds.DynamicData.DataReader(subscriber, topic)

        # Obtain the DataReader's Status Condition
        status_condition = dds.StatusCondition(reader)

        # Enable the 'data available' status.
        status_condition.enabled_statuses = dds.StatusMask.data_available()

        # Initialize samples_read to zero
        samples_read = 0
        
        # Associate a handler with the status condition. This will run when the
        # condition is triggered, in the context of the dispatch call (see below)
        def hander(_):  # condition argument is not used
            nonlocal samples_read
            nonlocal reader
            samples_read += process_data(reader)

        status_condition.handler(hander)

        # Create a WaitSet and attach the StatusCondition
        waitset = dds.WaitSet()
        waitset += status_condition

        # Catch control c interrupt
        try:
            while samples_read < sample_count:
                # Dispatch will call the handlers associated to the WaitSet conditions
                # when they activate
                print(f"Hello World subscriber sleeping for 4 seconds...")

                waitset.dispatch(dds.Duration(4))  # Wait up to 4s each time
        except KeyboardInterrupt:
            pass


if __name__ == "__main__":

    # Parse args
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
        default=10,
        dest="sample_count",
    )
    args = parser.parse_args()

    run_example(args.domain_id, args.sample_count)
