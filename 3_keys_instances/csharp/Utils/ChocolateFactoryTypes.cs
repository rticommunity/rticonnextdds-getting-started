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

using Rti.Dds.Core;
using Rti.Types.Dynamic;

namespace Utils
{
    /// <summary>
    /// Helper class that loads the types defined in chocolate_factory.xml
    /// </summary>
    public class ChocolateFactoryTypes
    {
        private readonly QosProvider provider =
            new QosProvider("../chocolate_factory.xml");

        public ChocolateFactoryTypes()
        {
            Temperature = (StructType)provider.GetType("Temperature");
            ChocolateLotState = (StructType)provider.GetType("ChocolateLotState");
            StationKind = (EnumType)provider.GetType("StationKind");
        }

        public StructType Temperature { get; }
        public StructType ChocolateLotState { get; }
        public EnumType StationKind { get; }
    }
}
