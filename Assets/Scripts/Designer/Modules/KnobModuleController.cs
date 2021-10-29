using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Events;

public class KnobModuleController : ModuleParent
{
    [SerializeField] private InputField minInput, maxInput, labelInput;

    public double min, max;
    public string Label { get; private set; }

    public UnityEvent<string> onLabelChanged = new UnityEvent<string>();
    public UnityEvent<double> onMinChanged = new UnityEvent<double>();
    public UnityEvent<double> onMaxChanged = new UnityEvent<double>();
    public UnityEvent<float> onSliderMoved = new UnityEvent<float>();

    private CircleSlider _circleSlider;

    protected override void ChildAwake()
    {
        _circleSlider = GetComponentInChildren<CircleSlider>();
        _circleSlider.onValueChanged.AddListener(onSliderMoved.Invoke);
        
        moduleType = ModuleType.KnobModule;
        minInput.onValueChanged.AddListener(val =>
        {
            min = double.Parse(val);
            _circleSlider.SetRange((float) min, _circleSlider.Max, true);
            onMinChanged.Invoke(min);
        });
        maxInput.onValueChanged.AddListener(val =>
        {
            max = double.Parse(val);
            _circleSlider.SetRange(_circleSlider.Min, (float) max, true);
            onMaxChanged.Invoke(max);
        });
        labelInput.onValueChanged.AddListener(val =>
        {
            Label = val;
            onLabelChanged.Invoke(Label);
        });
    }

    public void InvokeSliderCallback()
    {
        _circleSlider.onValueChanged.Invoke(_circleSlider.Value);
    }

    public void SetValues(string label, double mn, double mx)
    {
        Label = label;
        labelInput.text = Label;
        min = mn;
        minInput.text = min.ToString();
        max = mx;
        maxInput.text = max.ToString();
    }

    public void SetOutputIndex(int idx)
    {
        connectors[0].SetOutputIndex(idx);
    }

    public override List<ModuleConnectorController> GetUsedOutputs(out bool found)
    {
        var res = new List<ModuleConnectorController>();
        if (connectors[0].isConnected)
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
