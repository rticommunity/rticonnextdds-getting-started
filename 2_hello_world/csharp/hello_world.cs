
/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from hello_world.idl
using RTI Code Generator (rtiddsgen) version 3.1.0.
The rtiddsgen tool is part of the RTI Connext DDS distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the Code Generator User's Manual.
*/

using System;
using System.Reflection;
using System.Collections.Generic;
using Omg.Types;
using System.Linq;

public class HelloWorld : IEquatable<HelloWorld>
{
    public sHelloWorld()
    {
    }

    public HelloWorld( int msgParam)
    {
        msg = msgParam;
    }

    public HelloWorld(HelloWorld other_)
    {
        msg = other_.msg;
    }

    public int msg { get; set; }

    public bool Equals(HelloWorld other)
    {
        if (other == null)
        {
            return false;
        }

        if (ReferenceEquals(this, other))
        {
            return true;
        }
        return msg.Equals(other.msg) ;
    }

    public override bool Equals(object obj) => this.Equals(obj as HelloWorld);

    public override string ToString()
    {
        String result = new String("");
        foreach (PropertyInfo property in this.GetType().GetProperties())
        {
            result += result.Length == 0 ? "[" : ", ";
            result += property.Name +  ": ";
            result += property.GetValue(this, null) != null ? property.GetValue(this, null).ToString() : "null";
        }
        result +="]";
        return result;
    }

    public override int GetHashCode()
    {
        HashCode hash = new HashCode();

        hash.Add(msg);

        return hash.ToHashCode();
    }
}

