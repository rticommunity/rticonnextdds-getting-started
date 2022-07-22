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


import random
import argparse
import time

import rti.connextdds as dds
from chocolate_factory import Temperature, ChocolateLotState

def run_example(domain_id, sample_count, sensor_id):

    # A DomainParticipant allows an application to begin communicating in
    # a DDS domain. Typically there is one DomainParticipant per application.
    # DomainParticipant QoS is configured in USER_QOS_PROFILES.xml
    participant = dds.DomainParticipant(domain_id)

    # A Topic has a name and a datatype. Create a Topic named
    # "ChocolateTemperature" with type Temperature
    topic = dds.Topic(participant, "ChocolateTemperature", Temperature)

    # Exercise #2.1: Add a new Topic


    # A Publisher allows an application to create one or more DataWriters
    # Publisher QoS is configured in USER_QOS_PROFILES.xml
    publisher = dds.Publisher(participant)

    # This DataWriter writes data on Topic "ChocolateTemperature"
    # DataWriter QoS is configured in USER_QOS_PROFILES.xml
    writer = dds.DataWriter(publisher, topic)

    # Exercise #2.2: Add new DataWriter and data sample

    # Create data sample for writing
    sample = Temperature()
    i = 0
    try:
        while sample_count is None or i < sample_count:
            # Modify the data to be written here
            sample.sensor_id = sensor_id
            # random number x where 30 <= x <= 32
            sample.degrees = random.randint(30, 32)

            # Exercise #2.3: Write data with new ChocolateLotState DataWriter

            print(f"Writing ChocolateTemperature, count {i}")
            writer.write(sample)

            # Exercise 1.1: Change this to sleep 100 ms in between writing
            # temperatures
            time.sleep(4)

            i += 1

    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    # Parse the argument list
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
