/*
 * (c) Copyright, Real-Time Innovations, 2020.  All rights reserved.
 * RTI grants Licensee a license to use, modify, compile, and create derivative
 * works of the software solely for use with RTI Connext DDS. Licensee may
 * redistribute copies of the software provided that all such copies are subject
 * to this license. The software is provided "as is", with no warranty of any
 * type, including any warranty for fitness for any purpose. RTI is under no
 * obligation to maintain or support the software. RTI shall not be liable for
 * any incidental or consequential damages arising out of the use or inability
 * to use the software.
 */

using System;
using Omg.Dds.Core;
using Rti.Dds.Core;
using Rti.Dds.Core.Status;
using Rti.Dds.Subscription;
using Rti.Types.Dynamic;

namespace HelloWorld
{
    /// <summary>
    /// Example subscriber application
    /// </summary>
    public static class HelloWorldSubscriber
    {
        private static int ProcessData(DataReader<DynamicData> reader)
        {
            int samplesRead = 0;
            using var samples = reader.Take();
            foreach (var sample in samples.ValidData)
            {
                Console.WriteLine($"Received: {sample}");
                samplesRead++;
            }

            return samplesRead;
        }

        /// <summary>
        /// Main function, receiving structured command-line arguments
        /// </summary>
        /// <param name="sampleCount">The number of data samples to receive before exiting</param>
        public static void Main(int sampleCount = 10)
        {
            var provider = new QosProvider("../hello_world.xml");
            using var participant = provider.CreateParticipantFromConfig(
                "participants::hello_world_participant");

            var reader = participant.ImplicitSubscriber.LookupDataReader<DynamicData>(
                "ExampleHelloWorld")
                ?? throw new Exception("reader not found");

            var statusCondition = reader.StatusCondition;
            statusCondition.EnabledStatuses = StatusMask.DataAvailable;
            int samplesRead = 0;
            statusCondition.Triggered += _ => samplesRead += ProcessData(reader);

            var waitset = new WaitSet();
            waitset.AttachCondition(statusCondition);
            while (samplesRead < sampleCount)
            {
                Console.WriteLine("HelloWorld subscriber sleeping for 4 sec...");
                waitset.Dispatch(Duration.FromSeconds(4));
            }
        }
    }
}
