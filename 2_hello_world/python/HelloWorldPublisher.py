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
import argparse  # for argument parsing
import time  # for sleeping
import pathlib  # for getting the absolute path of this file to find the XML file

FILE = str(pathlib.Path(__file__).parent.absolute()) + "/../hello_world.xml"


def run_example(domain_id, sample_count):

    # A DomainParticipant allows an application to begin communicating in
    # a DDS domain. Typically there is one DomainParticipant per application.
    # DomainParticipant QoS is configured in USER_QOS_PROFILES.xml
    with dds.DomainParticipant(domain_id) as participant:
        # Here we use a provider because it allows us to easily extract the
        # type from the XML file. The definition for the type can be found
        # in HelloWorld.xml
        provider = dds.QosProvider(FILE)

        provider_type = provider.type("HelloWorld")

        # A Topic has a name and a datatype. Create a Topic named
        # "HelloWorld Topic" with type HelloWorld
        topic = dds.DynamicData.Topic(
            participant, "Example HelloWorld", provider_type
        )

        # A Publisher allows an application to create one or more DataWriters
        # Publisher QoS is configured in USER_QOS_PROFILES.xml
        publisher = dds.Publisher(participant)

        # This DataWriter will write data on Topic "HelloWorld Topic"
        # DataWriter QoS is configured in USER_QOS_PROFILES.xml
        writer = dds.DynamicData.DataWriter(publisher, topic)

        # Create data sample for writing
        sample = dds.DynamicData(provider_type)
        for i in range(0, sample_count):
            # Modify the data to be written here

            sample["msg"] = f"Hello {i}"

            print(f"Writing {sample['msg']}")

            writer.write(sample)
            time.sleep(1)


if __name__ == "__main__":
    # Parse args here
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
    # Check for control c
    try:
        run_example(args.domain_id, args.sample_count)
    except KeyboardInterrupt:
        pass
