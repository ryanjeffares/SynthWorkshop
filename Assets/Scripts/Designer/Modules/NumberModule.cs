using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class NumberModule : ModuleParent
{
    [SerializeField] private InputField _numberInput;

    private double _currentValue;
    private int _inputIndex = -1;
    private int _outputIndex = -1;

    protected override void ChildAwake()
    {
        moduleType = ModuleType.NumberBox;
        _numberInput.onValueChanged.AddListener(val =>
        {
            _currentValue = double.Parse(val);
            if(_outputIndex != -1)
            {
                SynthWorkshopLibrary.SetCvParam(_outputIndex, (float)_currentValue);
            }
        });
    }

    public override List<ModuleException> CheckErrors()
    {
        var exceptions = new List<ModuleException>();
        if(connectors.All(c => !c.isConnected))
        {
            var exception = new ModuleException("Number box not connected to anything and will be ignored.", ModuleException.SeverityLevel.Warning);
            exceptions.Add(exception);
        }
        if (!connectors[1].isConnected)
        {
            var exception = new ModuleException("Number box's output is not connected to anything, and will function as a display only.", ModuleException.SeverityLevel.Warning);
            exceptions.Add(exception);
        }
        return exceptions;
    }

    public Dictionary<string, object> CreateJsonEntry(Dictionary<ModuleConnectorController, int> outputLookup)
    {
        var res = new Dictionary<string, object>();
        if (connectors[0].isConnected)
        {
            _inputIndex = outputLookup[connectors[0].sourceConnector];
            res.Add("inputFrom", _inputIndex);
        }
        if (connectors[1].isConnected)
        {
            _outputIndex = outputLookup[connectors[1]];
            res.Add("outputTo", _outputIndex);
        }
        res.Add("initialValue", _currentValue);
        return res;
    }

    private void Update()
    {
        if (_inputIndex != -1)
        {
            _currentValue = SynthWorkshopLibrary.GetCvParam(_inputIndex);
            _numberInput.text = _currentValue.ToString();
        }
        if (_outputIndex != -1)
        {
            SynthWorkshopLibrary.SetCvParam(_outputIndex, (float)_currentValue);
        }
    }
}
