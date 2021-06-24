using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public class ADSRModuleController : ModuleParent
{
    protected override void ChildAwake()
    {
        moduleType = ModuleType.ADSR;
    }
    
    public override List<ModuleException> CheckErrors()
    {
        var exceptions = new List<ModuleException>();
        if (!connectors[0].isConnected)
        {
            var e = new ModuleException("ADSR has no audio input and will have no function.", ModuleException.SeverityLevel.Warning);
            exceptions.Add(e);
        }
        if (!connectors.Last().isConnected)
        {
            var e = new ModuleException("ADSR has no audio output and will be ignored.", ModuleException.SeverityLevel.Warning);
            exceptions.Add(e);
        }
        if (!connectors[5].isConnected)
        {
            var e = new ModuleException("ADSR has no trigger input and will have no function.", ModuleException.SeverityLevel.Warning);
            exceptions.Add(e);
        }
        var ins = new string[] {"Attack", "Decay", "Sustain", "Release"};
        for (int i = 0; i < 4; i++)
        {
            if (!connectors[i + 1].isConnected)
            {
                var e = new ModuleException($"ADSR has no input for {ins[i]}. You must set an input for {ins[i]}", ModuleException.SeverityLevel.Error);
                exceptions.Add(e);
            }
        }
        return exceptions;
    }

    public void CreateJsonEntry(Dictionary<string, object> jsonDict,
        Dictionary<ModuleConnectorController, int> outputLookup)
    {
        if (connectors.First().isConnected)
        {
            var inputNums = new List<int>();
            foreach (var input in connectors.First().connectedModuleConnectors)
            {
                inputNums.Add(outputLookup[input]);
            }
            jsonDict.Add("inputFrom", inputNums);
        }
        jsonDict.Add("attackFrom", outputLookup[connectors[1].sourceConnector]);
        jsonDict.Add("decayFrom", outputLookup[connectors[2].sourceConnector]);
        jsonDict.Add("sustainFrom", outputLookup[connectors[3].sourceConnector]);
        jsonDict.Add("releaseFrom", outputLookup[connectors[4].sourceConnector]);
        if (connectors[5].isConnected)
        {
            jsonDict.Add("triggerInput", outputLookup[connectors[5].sourceConnector]);
        }

        if (connectors.Last().isConnected)
        {
            jsonDict.Add("outputTo", outputLookup[connectors.Last()]);
        }
    }
}
