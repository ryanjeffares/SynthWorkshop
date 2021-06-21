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

    private bool _minSet, _maxSet;

    protected override void ChildAwake()
    {
        moduleType = ModuleType.ControlModule;
        minInput.onValueChanged.AddListener(val => { min = double.Parse(val); _minSet = true; });
        maxInput.onValueChanged.AddListener(val => { max = double.Parse(val); _maxSet = true; });
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

    public override List<ModuleException> CheckErrors()
    {
        var exceptions = new List<ModuleException>();
        if (string.IsNullOrEmpty(Label))
        {
            var exception = new ModuleException("Knob has no label, and will be hard to identify.", ModuleException.SeverityLevel.Warning);
            exceptions.Add(exception);
        }
        if (string.IsNullOrEmpty(minInput.text))
        {
            var exception = new ModuleException("Knob has no minimum set, exporting may not proceed. You must set a minimum.", ModuleException.SeverityLevel.Error);
            exceptions.Add(exception);
        }
        if (string.IsNullOrEmpty(maxInput.text))
        {
            var exception = new ModuleException("Knob has no maximum set, exporting may not proceed. You must set a maximum.", ModuleException.SeverityLevel.Error);
            exceptions.Add(exception);
        }
        return exceptions;
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
