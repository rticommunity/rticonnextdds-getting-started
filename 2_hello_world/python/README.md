# Introduction to Publish/Subscribe (Python3)
**Important: The new RTI Connext DDS Python API used in this example is experimental 
and may change**

# Hello World!
Just like in most programming tutorials, we're going to start with a Hello World example.
The type we'll be using is defined in hello_world.idl. Since the python version does not
support static types, we can convert this to an xml file by running
`rtiddsgen -convertToXml ../hello_world.idl`.

## Setup
This example requires a *Connext DDS* installation to run the *RTI Code Generator (rtiddsgen)*
(see next section). You must also have an installation of *RTI Connext DDS Python* which can
be found [here](https://github.com/rticommunity/connextdds-py). 

You will also need a valid license. You can learn how to set this up in the
[Getting Started Guide](https://community.rti.com/static/documentation/connext-dds/6.0.1/doc/manuals/connext_dds/getting_started/index.html)


## Running the examples
To run, open two terminals. In one terminal run the publisher and in the other run the 
subscriber. In one terminal run
```shell
$ python3 HelloWorldPublisher.py
```

In the other, run
```shell
$ python3 HelloWorldSubscriber.py
```
