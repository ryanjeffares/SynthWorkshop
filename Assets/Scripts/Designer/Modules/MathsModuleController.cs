using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;

public enum MathsSign
{
    Plus, Minus, Multiply, Divide
}

public class MathsModuleController : ModuleParent
{
    public MathsSign mathsSign;
    public AudioCV audioCV;

    private AudioCV _incomingSignalType;

    protected override void ChildAwake()
    {
        moduleType = ModuleType.MathsModule;
    }

    public void SetType(MathsSign sign, AudioCV acv)
    {
        mathsSign = sign;
        audioCV = acv;
        nameText.text = mathsSign switch
        {
            MathsSign.Plus => audioCV == AudioCV.Audio ? "+~" : "+",
            MathsSign.Minus => audioCV == AudioCV.Audio ? "-~" : "-",
            MathsSign.Multiply => audioCV == AudioCV.Audio ? "*~" : "*",
            MathsSign.Divide => audioCV == AudioCV.Audio ? "/~" : "/",
            _ => "Error Type"
        };
    }

    public void SetAudioOrCvIncoming(AudioCV acv, ModuleConnectorController moduleConnector)
    {
        if (moduleConnector == connectors[0]) return;
        _incomingSignalType = acv;
    }

    public void CreateJsonEntry(Dictionary<string, object> jsonDict, Dictionary<ModuleConnectorController, int> outputLookup)
    {
        jsonDict.Add("operator", mathsSign.ToString());
        if (connectors[0].isConnected)
        {
            jsonDict.Add("LeftInputFrom", outputLookup[connectors[0].sourceConnector]);
        }
        if (connectors[1].isConnected) 
        { 
            jsonDict.Add("RightInputFrom", outputLookup[connectors[1].sourceConnector]); 
        }
        jsonDict.Add("IncomingSignalType", _incomingSignalType.ToString());
        if (outputLookup.ContainsKey(connectors[2]))
        {
            jsonDict.Add("outputId", outputLookup[connectors[2]]);
        }
    }

    public override List<ModuleException> CheckErrors()
    {
        var exceptions = new List<ModuleException>();
        if (connectors.All(c => !c.isConnected))
        {
            var exception = new ModuleException("Maths node not connected to anything and will be ignored.", ModuleException.SeverityLevel.Warning);
            exceptions.Add(exception);
        }
        if (!connectors[1].isConnected)
        {
            var exception = new ModuleException("Maths node's output is not connected to anything, and will have no function.", ModuleException.SeverityLevel.Warning);
            exceptions.Add(exception);
        }
        return exceptions;
    }
}
