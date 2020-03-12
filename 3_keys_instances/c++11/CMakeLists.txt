# (c) 2020 Copyright, Real-Time Innovations, Inc. All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or revisions thereof,
# must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.

cmake_minimum_required(VERSION 3.7)
project(rticonnextdds-getting-started-keys-instances)

# Add the folder where the FindRTIConnextDDS.cmake file is placed to the
# CMAKE_MODULE_PATH variable
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_CURRENT_SOURCE_DIR}/../../resources/cmake"
)

# Find the ConnextDDS libraries. This will look fo the core and API libraries
# only
find_package(RTIConnextDDS
    "6.0.1"
    REQUIRED
    COMPONENTS
        core
)

# Run Codegen to generate the source code files for the types. Output the
# generated type files along with other source files.
include(ConnextDdsCodegen)
connextdds_rtiddsgen_run(
    IDL_FILE
        "${CMAKE_CURRENT_SOURCE_DIR}/../chocolate_factory.idl"
    OUTPUT_DIRECTORY
        "${CMAKE_CURRENT_SOURCE_DIR}"
    LANG C++11
)

add_custom_target(codegen_sources
    DEPENDS
        ${chocolate_factory_CXX11_GENERATED_SOURCES}
)

# Create the target for the publisher executable
add_executable(tempering_application
    "${CMAKE_CURRENT_SOURCE_DIR}/tempering_application.cxx"
    ${chocolate_factory_CXX11_GENERATED_SOURCES}
)

set_property(
    TARGET tempering_application
    PROPERTY CXX_STANDARD 11)


# Link against the ConnextDDS libraries
target_link_libraries(
    tempering_application
    PRIVATE
        RTIConnextDDS::cpp2_api
)

# Include the folder with the type header files
target_include_directories(tempering_application
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Create the target for the subscriber executable
add_executable(monitoring_ctrl_application
    "${CMAKE_CURRENT_SOURCE_DIR}/monitoring_ctrl_application.cxx"
    ${chocolate_factory_CXX11_GENERATED_SOURCES}
)

# Link against the ConnextDDS libraries
target_link_libraries(
    monitoring_ctrl_application
    PRIVATE
        RTIConnextDDS::cpp2_api
)
# Include the folder with the type header files
target_include_directories(monitoring_ctrl_application
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

set_property(
    TARGET monitoring_ctrl_application
    PROPERTY CXX_STANDARD 11)

# Copy the USER_QOS_PROFILES.xml to the location where you will be running
# so it is in the current working directory
add_custom_target(copy_qos
    DEPENDS
        "${CMAKE_CURRENT_BINARY_DIR}/USER_QOS_PROFILES.xml"
)

add_custom_command(
    OUTPUT
        "${CMAKE_CURRENT_BINARY_DIR}/USER_QOS_PROFILES.xml"
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/USER_QOS_PROFILES.xml"
            "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Copying USER_QOS_PROFILES.xml"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/USER_QOS_PROFILES.xml"
    VERBATIM
)

# Copy the USER_QOS_PROFILES.XML when the tempering application is built, and
# ensure Codegen is called
add_dependencies(tempering_application
    copy_qos
    codegen_sources
)

# Copy the USER_QOS_PROFILES.XML when the monitoring and control application is
# built and ensure Codegen is called
add_dependencies(monitoring_ctrl_application
    copy_qos
    codegen_sources
)
