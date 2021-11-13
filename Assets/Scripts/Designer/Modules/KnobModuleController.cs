using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Events;

public class KnobModuleController : ModuleParent
{
    [SerializeField] private TextInputField minInput, maxInput;

    public double min, max;

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
    }

    public void InvokeSliderCallback()
    {
        _circleSlider.onValueChanged.Invoke(_circleSlider.Value);
    }

    public void SetValues(string label, double mn, double mx)
    {
        min = mn;
        minInput.SetText(min.ToString(), false);
        max = mx;
        maxInput.SetText(max.ToString(), false);
    }

    public void SetValues(double mn, double mx, double val)
    {
        min = mn;
        minInput.SetText(min.ToString(), false);
        max = mx;
        maxInput.SetText(max.ToString(), false);

        _circleSlider.SetRange((float)min, (float)max, false);
        _circleSlider.Set((float)val, false);
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
        if (string.IsNullOrEmpty(minInput.GetText()))
        {
            var exception = new ModuleException("Knob has no minimum set, exporting may not proceed. You must set a minimum.", ModuleException.SeverityLevel.Error);
            exceptions.Add(exception);
        }
        if (string.IsNullOrEmpty(maxInput.GetText()))
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
