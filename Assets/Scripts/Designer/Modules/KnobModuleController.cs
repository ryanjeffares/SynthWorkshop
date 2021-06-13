using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class KnobModuleController : ModuleParent
{
    [SerializeField] private InputField minInput, maxInput, labelInput;

    public double min, max;

    private string _label;
    public string Label
    {
        get => _label;
    }
    
    protected override void ChildAwake()
    {
        moduleType = ModuleType.ControlModule;
        minInput.onValueChanged.AddListener(val => min = double.Parse(val));
        maxInput.onValueChanged.AddListener(val => max = double.Parse(val));
        labelInput.onValueChanged.AddListener(val => _label = val);
    }

    public override List<ModuleConnectorController> GetUsedOutputs(out bool found)
    {
        var res = new List<ModuleConnectorController>();
        if (CheckIfUsed())
        {
            res.Add(connectors[0]);
        }
        found = res.Count > 0;
        return res;
    }

    public Tuple<double, double> GetRange()
    {
        return new Tuple<double, double>(min, max);
    }

    public int GetOutputIdOfConnector(Dictionary<ModuleConnectorController, int> outputLookup)
    {
        return outputLookup[connectors[0]];
    }
}
