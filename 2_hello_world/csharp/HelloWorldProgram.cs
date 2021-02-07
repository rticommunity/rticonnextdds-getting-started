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

namespace HelloWorldExample
{
    /// <summary>
    /// Example application
    /// </summary>
    public static class HelloWorldProgram
    {
        /// <summary>
        /// Main function, receiving structured command-line arguments
        /// via the System.Console.DragonFruit package.
        /// For example: dotnet run -- --pub --domain-id 54 --sample-count 5
        /// </summary>
        /// <param name="pub">Whether to run a publisher or a subscriber (default)</param>
        /// <param name="domainId">The domain ID to create the DomainParticipant</param>
        /// <param name="sampleCount">The number of data samples to publish</param>
        public static void Main(bool pub = false, int domainId = 0, int sampleCount = int.MaxValue)
        {
            if (pub)
            {
                HelloWorldPublisher.RunPublisher(domainId, sampleCount);
            }
            else
            {
                HelloWorldSubscriber.RunSubscriber(domainId, sampleCount);
            }
        }
    }
}
