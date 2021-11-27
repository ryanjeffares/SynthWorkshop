using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class NumberModule : ModuleParent
{
    [SerializeField] private TextInputField numberInput;

    public double CurrentValue { get; private set; }
    private int _inputIndex = -1, _outputIndex = -1;

    protected override void ChildAwake()
    {
        moduleType = ModuleType.NumberBox;
        numberInput.onValueChanged.AddListener(text =>
        {
            if (double.TryParse(text, out var val))
            {
                CurrentValue = val;
                if(_outputIndex != -1)
                {
                    SynthWorkshopLibrary.SetNumberBoxValue(GlobalIndex, (float)CurrentValue);
                }
            }
        });
    }

    public void SetNumber(double val)
    {
        CurrentValue = val;
        numberInput.SetText(CurrentValue.ToString(), false);
    }

    public void SetOutputIndex(int idx)
    {
        _outputIndex = idx;
        connectors[1].SetOutputIndex(idx);
    }

    public void SetInputIndex(int idx)
    {
        _inputIndex = idx;
        connectors[0].isConnected = true;
    }
    
    public override List<ModuleException> CheckErrors()
    {
        var exceptions = new List<ModuleException>();
        if(connectors.All(c => !c.isConnected))
        {
            var exception = new ModuleException("Number box not connected to anything and will be ignored.", 
                ModuleException.SeverityLevel.Warning);
            exceptions.Add(exception);
        }
        if (!connectors[1].isConnected)
        {
            var exception = new ModuleException("Number box's output is not connected to anything, and will function as a display only.", 
                ModuleException.SeverityLevel.Warning);
            exceptions.Add(exception);
        }
        if (!connectors[0].isConnected && connectors[1].isConnected && string.IsNullOrEmpty(numberInput.GetText()))
        {
            var exception = new ModuleException("Number box's input is not connected to anything but its output is connected and its input field is empty. " +
                                                "You must set a value if the number box's output is used.", ModuleException.SeverityLevel.Error);
            exceptions.Add(exception);
        }
        return exceptions;
    }

    public Dictionary<string, object> CreateJsonEntry(Dictionary<ModuleConnectorController, int> outputLookup)
    {
        var res = new Dictionary<string, object>();
        _inputIndex = outputLookup[connectors[0].isConnected ? connectors[0].sourceConnector : connectors[1]];
        if (connectors[1].isConnected)
        {
            res.Add("output_to", _inputIndex);
        }
        res.Add("initial_value", CurrentValue);
        res.Add("position", transform.localPosition);
        return res;
    }
    
    // might be null!
    public ModuleConnectorController GetSourceConnector()
    {
        return connectors[0].sourceConnector;
    }
    
    private void Update()
    {
        if (_inputIndex == -1 || !connectors[0].isConnected) return;
        CurrentValue = SynthWorkshopLibrary.GetCvParam(_inputIndex);
        numberInput.SetText(CurrentValue.ToString(), false);
    }
}
