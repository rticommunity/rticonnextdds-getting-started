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
using System.Threading;
using Rti.Dds.Core;
using Rti.Types.Dynamic;

namespace HelloWorld
{
    /// <summary>
    /// Example publisher application
    /// </summary>
    public static class HelloWorldPublisher
    {
        /// <summary>
        /// Main function, receiving structured command-line arguments
        /// </summary>
        /// <param name="sampleCount">The number of data samples to publish</param>
        public static void Main(int sampleCount = 10)
        {
            var provider = new QosProvider("../hello_world.xml");
            using var participant = provider.CreateParticipantFromConfig(
                "participants::hello_world_participant");

            var writer = participant.ImplicitPublisher.LookupDataWriter<DynamicData>(
                "ExampleHelloWorld")
                ?? throw new Exception("writer not found");

            var sample = writer.CreateData();
            for (int count = 0; count < sampleCount; count++)
            {
                // Modify the data to be written here
                sample.SetValue("msg", $"Hello {count}");

                Console.WriteLine($"Writing {sample}");
                writer.Write(sample);

                Thread.Sleep(1000);
            }
        }
    }
}
