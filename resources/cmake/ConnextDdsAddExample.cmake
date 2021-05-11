# (c) 2017 Copyright, Real-Time Innovations, Inc.  All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or revisions thereof,
# must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.

#[[.rst:
.. connextdds_add_example:

ConnextDdsAddExample
--------------------

Function to build the examples.

::

    connextdds_add_example(
        IDL name
        LANG language
        [PREFIX prefix]
        [NO_REQUIRE_QOS]
        [REQUIRE_SCRIPT]
        [DEPENDENCIES ...]
        [CODEGEN_ARGS ...]
        [APPLICATION ...]

    )

Add a new example to be built. This method will call Codegen, create the
CMake targets and set the dependencies.

``IDL`` (required):
    Name of the IDL file (without extension).
``LANG`` (required):
    Example language. Valid values are ``C``, ``C++``, ``C++03`` and ``C++11``.
``PREFIX``:
    Prefix name for the targets. If not present, the folder of the example name
    will be used as prefix.
``DISABLE_SUBSCRIBER``:
    Disable the subscriber build. The target for the subcriber will not be
    created.
``NO_REQUIRE_QOS``:
    If present, the function will copy the USER_QOS_PROFILES.xml file from the
    source directory to the binary directory.
``REQUIRE_SCRIPT``:
    If present, the function will copy the start_all.sh/bat file from the
    source directory to the binary directory.
``DEPENDENCIES``:
    Other target dependencies.
``CODEGEN_ARGS``:
    Extra arguments for Codegen.

Output targets:
``<prefix>_publisher_<lang>``
    Target for the publisher application.
``<prefix>_subscriber_<lang>``
    Target for the subscriber application.
``<prefix>_<lang>_sources``
    Target to call Codegen to generate the source files for the types and the
    publisher and the subscriber (if they were generated).
``<prefix>_<lang>_obj``
    Target for the object library with all the objects created with the source
    code for the types.
``<prefix>_<lang>_qos``
    Target to copy the USER_QOS_PROFILES.xml (if exists).


connextdds_call_codegen
--------------------

Function to generate the source code from codegen.

::

    connextdds_call_codegen(
        IDL name
        LANG language
        [PREFIX prefix]
        [CODEGEN_ARGS ...]
    )

Add a new example to be built. This method will call Codegen, create the
CMake targets and set the dependencies.

``IDL`` (required):
    Name of the IDL file (without extension).
``LANG`` (required):
    Example language. Valid values are ``C``, ``C++``, ``C++03`` and ``C++11``.
``PREFIX``:
    Prefix name for the targets. If not present, the folder of the example name
    will be used as prefix.
``CODEGEN_ARGS``:
    Extra arguments for Codegen.


connextdds_copy_qos_profile
---------------------------

Function to copy the USER_QOS_PROFILES.xml file.

::

    connextdds_copy_qos_profile(
        TARGET_PREFIX prefix
        DEPENDANT_TARGET dependant_target
        [FILENAME qos_filename]
    )

Copy the QoS profile file.  It will create the ``<prefix>_<lang>_qos``
target. It will make the given target dependant of the target for the QoS.

``TARGET_PREFIX (required)``:
    Prefix name for the target.
``DEPENDANT_TARGET``:
    This target will depends of the created QoS target. So, when the dependant
    target is build, the QoS file will be copied.
``FILENAME``:
    The filename of the QoS file to be copied.  If not specified the default
    name USER_QOS_PROFILES.xml will be used.

connextdds_add_application
--------------------------

Function to build an example.

::

    connextdds_add_application(
        TARGET target
        LANG language

    connextdds_add_application(
        TARGET target
        LANG language
        SOURCES ...
        [PREFIX prefix]
        [OUTPUT_NAME output_name]
        [QOS_FILENAME]
        [NO_REQUIRE_QOS]
        [REQUIRE_SCRIPT]
        [DEPENDENCIES ...]
    )

This method will create the executables from chosen sources.

``TARGET`` (required):
    Name of the target for the application.
``LANG`` (required):
    Example language. Valid values are ``C``, ``C++``, ``C++03`` and ``C++11``.
``PREFIX``:
    Prefix name for the targets. If not present, the folder of the example name
    will be used as prefix.
``OUTPUT_NAME``:
    Output name for the application. If not present, the target name will be
    used.
``QOS_FILENAME``:
    The filename of the desired QOS file.  If not specified USER_QOS_PROFILES.xml 
    is used.
``NO_REQUIRE_QOS``:
    If present, the QoS file will not be copied to the binary dir.
``REQUIRE_SCRIPT``:
    If present, the start_all.sh/bat file will be copied to the binary dir.
``DEPENDENCIES``:
    Other libraries to link.

#]]

include_guard(DIRECTORY)

# Find the RTI Connext DDS libraries
if(NOT RTIConnextDDS_FOUND)
    find_package(RTIConnextDDS
        "6.1.0"
        REQUIRED
        COMPONENTS
            core
    )
endif()

include(CMakeParseArguments)
include(ConnextDdsArgumentChecks)
include(ConnextDdsCodegen)


function(connextdds_add_example)
    set(optional_args DISABLE_SUBSCRIBER NO_REQUIRE_QOS REQUIRE_SCRIPT)
    set(single_value_args IDL LANG PREFIX QOS_FILENAME)
    set(multi_value_args CODEGEN_ARGS DEPENDENCIES APPLICATION_NAMES)
    cmake_parse_arguments(_CONNEXT
        "${optional_args}"
        "${single_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )
    connextdds_check_required_arguments(
        _CONNEXT_IDL
        _CONNEXT_LANG
    )

    # First, we call Codegen to generate the header and source files from the
    # IDL
    if(_CONNEXT_CODEGEN_ARGS)
        set(codegen_args CODEGEN_ARGS ${_CONNEXT_CODEGEN_ARGS})
    else()
        set(codegen_args)
    endif()

    if(_CONNEXT_PREFIX)
        set(prefix "${_CONNEXT_PREFIX}")
    else()
        # If no prefix was given, the name of the example folder name is used
        # as prefix
        get_filename_component(
            folder_name
            "${CMAKE_CURRENT_SOURCE_DIR}"
            DIRECTORY)
        get_filename_component(
            folder_name
            "${folder_name}"
            NAME)
        set(prefix "${folder_name}")
    endif()

    # Generate the sources for the types
    connextdds_call_codegen(
        IDL "${_CONNEXT_IDL}"
        IDL_REL_DIR ".."
        LANG "${_CONNEXT_LANG}"
        PREFIX "${prefix}"
        ${codegen_args}
    )

    # Set the name of the QoS file if required
    if(_CONNEXT_QOS_FILENAME)
        set(qos_filename QOS_FILENAME "${_CONNEXT_QOS_FILENAME}")
    else()
        set(qos_filename)
    endif()

    # Copy the USER_QOS_PROFILES.xml file if required
    if(_CONNEXT_NO_REQUIRE_QOS)
        set(no_require_qos NO_REQUIRE_QOS)
    else()
        set(no_require_qos)
    endif()

    # Copy the start_all script if required
    if(_CONNEXT_REQUIRE_SCRIPT)
        set(require_script REQUIRE_SCRIPT)
    else()
        set(require_script)
    endif()


    # We will use source code provided to build applications
    # in the repository for the application names specified
    connextdds_sanitize_language(LANG ${_CONNEXT_LANG} VAR lang_var)

    set(ex "c")
    if("${_CONNEXT_LANG}" STREQUAL "C++" OR
        "${_CONNEXT_LANG}" STREQUAL "C++03" OR
        "${_CONNEXT_LANG}" STREQUAL "C++11")
        set(ex "cxx")
    endif()
    foreach(_APPLICATION_NAME ${_CONNEXT_APPLICATION_NAMES})

        set(${_APPLICATION_NAME}_src
            "${CMAKE_CURRENT_SOURCE_DIR}/${_APPLICATION_NAME}.${ex}"
        )

        # Add the target for the publisher application
        connextdds_add_application(
            TARGET "${_APPLICATION_NAME}"
            LANG ${_CONNEXT_LANG}
            SOURCES ${${_APPLICATION_NAME}_src}
            PREFIX ${prefix}
            OUTPUT_NAME "${_APPLICATION_NAME}"
            ${qos_filename}
            ${no_require_qos}
            ${require_script}
            SOURCES
                $<TARGET_OBJECTS:${prefix}_${lang_var}_obj>
                "${${_APPLICATION_NAME}_src}"
        )
    endforeach(_APPLICATION_NAME)

endfunction()


function(connextdds_call_codegen)
    set(optional_args)
    set(single_value_args IDL IDL_REL_DIR LANG PREFIX)
    set(multi_value_args CODEGEN_ARGS)
    cmake_parse_arguments(_CONNEXT
        "${optional_args}"
        "${single_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )
    connextdds_check_required_arguments(
        _CONNEXT_IDL
        _CONNEXT_LANG
        _CONNEXT_PREFIX
    )

    # Get the list of the source files to generate and configure the command
    # to call Codegen
    connextdds_rtiddsgen_run(
        IDL_FILE
            "${CMAKE_CURRENT_SOURCE_DIR}/${_CONNEXT_IDL_REL_DIR}/${_CONNEXT_IDL}.idl"
        OUTPUT_DIRECTORY
            "${CMAKE_CURRENT_SOURCE_DIR}"
        LANG ${_CONNEXT_LANG}
        ${_CONNEXT_CODEGEN_ARGS}
    )

    # Get the path to the generated publisher and subscriber source code
    connextdds_sanitize_language(LANG ${_CONNEXT_LANG} VAR lang_var)
    set(${_CONNEXT_IDL}_${lang_var}_PUBLISHER_SOURCE
        "${${_CONNEXT_IDL}_${lang_var}_PUBLISHER_SOURCE}"
        PARENT_SCOPE
    )
    set(${_CONNEXT_IDL}_${lang_var}_SUBSCRIBER_SOURCE
        "${${_CONNEXT_IDL}_${lang_var}_SUBSCRIBER_SOURCE}"
        PARENT_SCOPE
    )

    # This will help to ensure that all the files generated by codegen are
    # created
    add_custom_target(${_CONNEXT_PREFIX}_${lang_var}_sources
        DEPENDS
            ${${_CONNEXT_IDL}_${lang_var}_GENERATED_SOURCES}
    )

    # If we don't use a object library to generate our publisher and subsciber
    # applications and we are building using more than one job, could happen
    # the following situation:
    # 1. The publisher and subscriber applications will be build in two
    #   different jobs (A and B)
    # 2. The source code for the types is not generated
    # 3. The job A will call Codegen
    # 4. The job A starts to build the application
    # 4. The job B calls Codegen
    # 5. The build performer by the job A is broken because the job B is
    #   replacing code generated by the job A.
    # Using the object library, we can depend of it and call the Codegen only
    # one time.
    set(obj_library ${_CONNEXT_PREFIX}_${lang_var}_obj)
    add_library(${obj_library}
        OBJECT
        ${${_CONNEXT_IDL}_${lang_var}_SOURCES}
    )

    set(api "c")
    set(c_standard C_STANDARD 90)
    set(cxx_standard CXX_STANDARD 98)
    if("${_CONNEXT_LANG}" STREQUAL "C++")
        set(api "cpp")
    elseif("${_CONNEXT_LANG}" STREQUAL "C++03")
        set(api "cpp2")
    elseif("${_CONNEXT_LANG}" STREQUAL "C++11")
        set(api "cpp2")
        set(cxx_standard CXX_STANDARD 11)
    endif()

    target_compile_definitions(
        ${obj_library}
        PRIVATE
        $<TARGET_PROPERTY:RTIConnextDDS::${api}_api,INTERFACE_COMPILE_DEFINITIONS>)

    target_include_directories(
        ${obj_library}
        PRIVATE
        $<TARGET_PROPERTY:RTIConnextDDS::${api}_api,INTERFACE_INCLUDE_DIRECTORIES>)

    set_target_properties(
        ${obj_library}
        PROPERTIES
            ${c_standard}
            ${cxx_standard}
    )

    add_dependencies(${obj_library}
        ${_CONNEXT_PREFIX}_${lang_var}_sources
    )

endfunction()


function(connextdds_copy_qos_profile)
    set(optional_args)
    set(single_value_args TARGET_PREFIX DEPENDANT_TARGET FILENAME)
    set(multi_value_args)

    cmake_parse_arguments(_EXAMPLE_QOS
        "${optional_args}"
        "${single_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )
message(STATUS "Example QoS filename: ${_EXAMPLE_QOS_FILENAME}")
    if(_EXAMPLE_QOS_FILENAME) 
        set(user_qos_profile_name ${_EXAMPLE_QOS_FILENAME})
    else()
        set(user_qos_profile_name "USER_QOS_PROFILES.xml")
    endif()

    set(qos_file "${CMAKE_CURRENT_SOURCE_DIR}/${user_qos_profile_name}")

    if(NOT EXISTS ${qos_file})
        message(FATAL_ERROR "The ${user_qos_profile_name} was not found")
    endif()

    set(output_qos "${CMAKE_CURRENT_BINARY_DIR}/${user_qos_profile_name}")

    set(target_qos "${_EXAMPLE_QOS_TARGET_PREFIX}qos")

    if(NOT TARGET ${target_qos})
        # We created a target and it depends of the output file
        add_custom_target(${target_qos}
            DEPENDS
                ${output_qos}
        )

        # Add the command to copy the USER_QOS_PROFILES.xml
        add_custom_command(
            OUTPUT
                ${output_qos}
            COMMAND
                ${CMAKE_COMMAND} -E copy_if_different
                    ${qos_file}
                    "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "Copying ${user_qos_profile_name}"
            DEPENDS
                ${qos_file}
            VERBATIM
        )
    endif()

    add_dependencies(${_EXAMPLE_QOS_DEPENDANT_TARGET}
        ${target_qos}
    )

endfunction()

function(connextdds_copy_script)
    set(optional_args)
    set(single_value_args TARGET_PREFIX DEPENDANT_TARGET FILENAME)
    set(multi_value_args)

    cmake_parse_arguments(_EXAMPLE_SCRIPT
        "${optional_args}"
        "${single_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )

    if(_EXAMPLE_SCRIPT) 
        set(script_name ${_EXAMPLE_SCRIPT})
    elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
        set(script_name "start_all.bat")
    elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
        set(script_name "start_all.sh")
    else()
        set(script_name "start_all.sh")
    endif()

    set(script_file "${CMAKE_CURRENT_SOURCE_DIR}/${script_name}")

    if(NOT EXISTS ${script_file})
        message(FATAL_ERROR "The ${script_name} file was not found")
    endif()

    set(output_script "${CMAKE_CURRENT_BINARY_DIR}/${script_name}")

    set(target_script "${_EXAMPLE_SCRIPT_TARGET_PREFIX}script")

    if(NOT TARGET ${target_script})
        # We created a target and it depends of the output file
        add_custom_target(${target_script}
            DEPENDS
                ${output_script}
        )

        # Add the command to copy the start_all script
        add_custom_command(
            OUTPUT
                ${output_script}
            COMMAND
                ${CMAKE_COMMAND} -E copy_if_different
                    ${script_file}
                    "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "Copying ${script_name}"
            DEPENDS
                ${script_file}
            VERBATIM
        )
    endif()
message(STATUS "script: ${_EXAMPLE_SCRIPT_DEPENDANT_TARGET} ${target_script}")
    add_dependencies(${_EXAMPLE_SCRIPT_DEPENDANT_TARGET}
        ${target_script}
    )

endfunction()

function(connextdds_add_application)
    set(optional_args NO_REQUIRE_QOS REQUIRE_SCRIPT)
    set(single_value_args TARGET LANG PREFIX OUTPUT_NAME QOS_FILENAME)
    set(multi_value_args SOURCES DEPENDENCIES)

    cmake_parse_arguments(_CONNEXT
        "${optional_args}"
        "${single_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )
    connextdds_check_required_arguments(
        _CONNEXT_TARGET
        _CONNEXT_LANG
        _CONNEXT_SOURCES
     )

    set(api "c")
    set(c_standard C_STANDARD 90)
    set(cxx_standard CXX_STANDARD 98)
    if("${_CONNEXT_LANG}" STREQUAL "C++")
        set(api "cpp")
    elseif("${_CONNEXT_LANG}" STREQUAL "C++03")
        set(api "cpp2")
    elseif ("${_CONNEXT_LANG}" STREQUAL "C++11")
        set(api "cpp2")
        set(cxx_standard CXX_STANDARD 11)
    endif()

    if(_CONNEXT_PREFIX)
        set(target_name "${_CONNEXT_PREFIX}_${_CONNEXT_TARGET}_${api}")
        set(qos_target "${_CONNEXT_PREFIX}_qos_${api}")
        set(qos_prefix "${_CONNEXT_PREFIX}_${api}")
        set(script_prefix "${_CONNEXT_PREFIX}_${api}")
    else()
        set(target_name "${_CONNEXT_TARGET}_${api}")
        set(qos_target "qos_${api}")
        set(qos_prefix "${api}")
        set(script_prefix "${api}")
    endif()

    # Create the target
    add_executable(${target_name} ${_CONNEXT_SOURCES})

    # Link all the dependencies and the RTI libraries for the API language
    target_link_libraries(${target_name}
        PRIVATE
            ${_CONNEXT_DEPENDENCIES}
            RTIConnextDDS::${api}_api
    )

    # Add the include directories
    target_include_directories(${target_name}
        PRIVATE
            "${CMAKE_CURRENT_BINARY_DIR}/src"
    )

    # Set output name for the binary
    set_target_properties(${target_name}
        PROPERTIES
            OUTPUT_NAME "${_CONNEXT_OUTPUT_NAME}"
            ${c_standard}
            ${cxx_standard}
    )

    if(NOT _CONNEXT_NO_REQUIRE_QOS)
        if (_CONNEXT_QOS_FILENAME)
            connextdds_copy_qos_profile(
                TARGET_PREFIX "${qos_prefix}_"
                DEPENDANT_TARGET "${target_name}"
                FILENAME ${_CONNEXT_QOS_FILENAME})
        else()
            connextdds_copy_qos_profile(
                TARGET_PREFIX "${qos_prefix}_"
                DEPENDANT_TARGET "${target_name}")
        endif()

    endif()

    if(_CONNEXT_REQUIRE_SCRIPT)
        if (_CONNEXT_SCRIPT)
            connextdds_copy_script(
                TARGET_PREFIX "${script_prefix}_"
                DEPENDANT_TARGET "${target_name}"
                FILENAME ${_CONNEXT_SCRIPT_FILENAME})
        else()
            connextdds_copy_script(
                TARGET_PREFIX "${script_prefix}_"
                DEPENDANT_TARGET "${target_name}")
        endif()

    endif()

endfunction()
