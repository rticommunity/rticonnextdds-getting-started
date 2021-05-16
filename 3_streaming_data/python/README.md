# Introduction to Data Types: streaming data (Python3)
**IMPORTANT: The new RTI Connext DDS Python API used in this example
is experimental and might change**

This code follows the "Introduction to Data Types" chapter of the
[RTI Connext DDS Getting Started Guide](https://community.rti.com/static/documentation/connext-dds/6.0.1/doc/manuals/connext_dds/getting_started/index.html).
Currently that guide walks you through the C++ examples, but you can follow it to
understand what the example does and the DDS concepts it demonstrates.

## Setup
This example requires a *Connext DDS* installation to run the *RTI Code Generator (rtiddsgen)*
(see next section). You must also have an installation of *RTI Connext DDS Python* which can
be found [here](https://github.com/rticommunity/connextdds-py).

To generate the XML definition from IDL, run
```shell
$ rtiddsgen -convertToXml ../chocolate_factory.idl
```

The example requires a valid license file, which can be configured with
the `RTI_LICENSE_FILE` environment variable. Follow the instructions under
"Setting Up a License" in the [Getting Started Guide](https://community.rti.com/static/documentation/connext-dds/6.0.1/doc/manuals/connext_dds/getting_started/index.html).

## Running the application
To run the examples, open two terminals.
In one, run
```shell
$ python3 ChocolateFactorySubscriber.py
```
In the other, run
```shell
$ python3 ChocolateFactoryPublisher.py
```
