
# (c) Copyright, Real-Time Innovations, 2022.  All rights reserved.
# RTI grants Licensee a license to use, modify, compile, and create derivative
# works of the software solely for use with RTI Connext DDS. Licensee may
# redistribute copies of the software provided that all such copies are subject
# to this license. The software is provided "as is", with no warranty of any
# type, including any warranty for fitness for any purpose. RTI is under no
# obligation to maintain or support the software. RTI shall not be liable for
# any incidental or consequential damages arising out of the use or inability
# to use the software.

import argparse
import sys
from hello_world_publisher import HelloWorldPublisher
from hello_world_subscriber import HelloWorldSubscriber
from dataclasses import dataclass

@dataclass
class ApplicationArguments:
    pub: bool
    sub: bool
    domain: int
    sample_count: int

def main():
    """ The Main function runs the publisher or the subscriber """
    args: ApplicationArguments = parse_arguments()

    if(args.pub):
        print(f"Running HelloWorldPublisher on domain {args.domain}")
        HelloWorldPublisher.run_publisher(
            domain_id=args.domain,
            sample_count=args.sample_count)
    else:
        print(f"Running HelloWorldSubscriber on domain {args.domain}")
        HelloWorldSubscriber.run_subscriber(
            domain_id=args.domain,
            sample_count=args.sample_count)

def check_sample_count_range(value):
    """ Check if the sample count is in the expected range """
    try:
        value = int(value)
        if value <= 0:
            raise argparse.ArgumentTypeError(
                f"The sample count ({value}) must be larger than 0")
    except ValueError:
        raise Exception(f"The sample count ({value}) must be an integer")

    return value

def check_domain_range(value):
    """ Check if the domain id is in the expected range """
    try:
        value = int(value)
        if value < 0:
            raise argparse.ArgumentTypeError(
                f"The domain id ({value}) must be equal to or larger than 0")
    except ValueError:
        raise Exception(f"The domain id ({value}) must be an integer")

    return value

def parse_arguments():

    """Uses the argparse library to parse the command line arguments. """
    parser = argparse.ArgumentParser(description='Runs HelloWorldPublisher or HelloWorldSubscriber')


    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        '-p',
        '--pub',
        help='Whether to run the publisher application',
        action='store_true')
    group.add_argument(
        '-s',
        '--sub',
        help='Whether to run the subscriber application',
        action='store_true')
    parser.add_argument(
        '-d',
        '--domain',
        help='Domain ID used to create the DomainParticipant',
        default=0,
        type=check_domain_range,
        metavar="[>=0]")
    parser.add_argument(
        '-c',
        '--sample-count',
        help='Number of samples to receive before closing',
        default=sys.maxsize,
        type=check_sample_count_range,
        metavar=("[1-" + str(sys.maxsize-1) +"]"))

    return parser.parse_args(namespace=ApplicationArguments)

if __name__ == '__main__':
    main()