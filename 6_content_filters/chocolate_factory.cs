
/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from chocolate_factory.idl
using RTI Code Generator (rtiddsgen) version ${envMap.codegenVersion}.
The rtiddsgen tool is part of the RTI Connext DDS distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the Code Generator User's Manual.
*/

using System;
using System.Reflection;
using System.Collections.Generic;
using Omg.Types;
using System.Linq;

public class Temperature : IEquatable<Temperature>
{
    public Temperature()
    {
    }

    public Temperature(int degreesParam)
    {
        degrees = degreesParam;
    }

    public Temperature(Temperature other_)
    {
        degrees = other_.degrees;
    }

    public int degrees { get; set; }

    public bool Equals(Temperature other)
    {
        if (other == null)
        {
            return false;
        }

        if (ReferenceEquals(this, other))
        {
            return true;
        }
        return degrees.Equals(other.degrees) ;
    }

    public override bool Equals(object obj) => this.Equals(obj as Temperature);

    public override string ToString()
    {
        String result = new String("");
        foreach (PropertyInfo property in this.GetType().GetProperties())
        {
            result += result.Length == 0 ? "[" : ", ";
            result += property.Name +  ": ";
            result += property.GetValue(this, null).ToString();
        }
        result +="]";
        return result;
    }

    public override int GetHashCode()
    {
        HashCode hash = new HashCode();

        hash.Add(degrees);

        return hash.ToHashCode();
    }
}
public class ChocolateLotState : IEquatable<ChocolateLotState>
{
    public ChocolateLotState()
    {
        station = new StationKind();
        next_station = new StationKind();
        lot_status = new LotStatusKind();
    }

    public ChocolateLotState(uint lot_idParam,StationKind stationParam,StationKind next_stationParam,LotStatusKind lot_statusParam)
    {
        lot_id = lot_idParam;
        station = stationParam;
        next_station = next_stationParam;
        lot_status = lot_statusParam;
    }

    public ChocolateLotState(ChocolateLotState other_)
    {
        lot_id = other_.lot_id;
        station = new StationKind(other_.station);
        next_station = new StationKind(other_.next_station);
        lot_status = new LotStatusKind(other_.lot_status);
    }

    public uint lot_id { get; set; }
    public StationKind station { get; set; }
    public StationKind next_station { get; set; }
    public LotStatusKind lot_status { get; set; }

    public bool Equals(ChocolateLotState other)
    {
        if (other == null)
        {
            return false;
        }

        if (ReferenceEquals(this, other))
        {
            return true;
        }
        return lot_id.Equals(other.lot_id) &&
        station.Equals(other.station) &&
        next_station.Equals(other.next_station) &&
        lot_status.Equals(other.lot_status) ;
    }

    public override bool Equals(object obj) => this.Equals(obj as ChocolateLotState);

    public override string ToString()
    {
        String result = new String("");
        foreach (PropertyInfo property in this.GetType().GetProperties())
        {
            result += result.Length == 0 ? "[" : ", ";
            result += property.Name +  ": ";
            result += property.GetValue(this, null).ToString();
        }
        result +="]";
        return result;
    }

    public override int GetHashCode()
    {
        HashCode hash = new HashCode();

        hash.Add(lot_id);
        hash.Add(station);
        hash.Add(next_station);
        hash.Add(lot_status);

        return hash.ToHashCode();
    }
}

