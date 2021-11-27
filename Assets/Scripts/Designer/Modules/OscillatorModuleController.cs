using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.UI;

public enum OscillatorType
{
    Sine, Saw, Pulse, Tri
}

public class OscillatorModuleController : ModuleParent
{
    public float Frequency { get; private set; }

    private OscillatorType _type;
    public OscillatorType Type
    {
        get => _type;
        set
        {
            _type = value;
            nameText.text = Type.ToString().ToLower() + '~';
            if (Type == OscillatorType.Pulse)
            {
                connectors[3].transform.parent.gameObject.SetActive(true);
            }
            else
            {
                Destroy(connectors[3].transform.parent.gameObject);
                connectors.RemoveAt(3);
            }
        }
    }

    protected override void ChildAwake()
    {
        moduleType = ModuleType.OscillatorModule;        
    }

    public void SetOutputIndexes(int soundOut, int cvOut)
    {
        connectors[0].SetOutputIndex(soundOut);
        connectors[1].SetOutputIndex(cvOut);
    }

    public void SetFrequency(float frequency)
    {
        Frequency = frequency;
        nameText.text = $"{Type.ToString().ToLower()}~ {Math.Round(Frequency, 2)}";
    }

    public override List<ModuleException> CheckErrors()
    {
        var exceptions = new List<ModuleException>();
        if (connectors.All(c => !c.isConnected))
        {
            var exception = new ModuleException("Oscillator node not connected to anything and will be ignored.", ModuleException.SeverityLevel.Warning);
            exceptions.Add(exception);
        }
        if (!connectors[1].isConnected && !connectors[0].isConnected)
        {
            var exception = new ModuleException("Oscillator's output is not connected to anything, and will have no function.", ModuleException.SeverityLevel.Warning);
            exceptions.Add(exception); 
        }
        if (!connectors[2].isConnected)
        {
            var exception = new ModuleException("Oscillator node has no frequency input and will be ignored.", ModuleException.SeverityLevel.Warning);
            exceptions.Add(exception);
        }
        return exceptions;
    }

    public void SetType(OscillatorType t)
    {
        Type = t;        
    }

    public void CreateJsonEntry(Dictionary<string, object> jsonDict, Dictionary<ModuleConnectorController, int> outputLookup)
    {
        var propertyNames = new[] { "sound_output_to", "cv_out_to", "freq_input_from", "pw_input_from" };
        for(int i = 0; i < connectors.Count; i++)
        {
            if (!connectors[i].isConnected) continue;
            if(connectors[i].inputOutput == InputOutput.Output)
            {
                jsonDict.Add(propertyNames[i], outputLookup[connectors[i]]);
            }
            else
            {
                jsonDict.Add(propertyNames[i], outputLookup[connectors[i].sourceConnector]);
            }
        }
        jsonDict.Add("position", transform.localPosition);
    }
}
