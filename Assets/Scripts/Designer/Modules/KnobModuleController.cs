using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Events;

public class KnobModuleController : ModuleParent
{
    [SerializeField] private TextInputField minInput, maxInput, skewInput;
    [SerializeField] private ToggleSwitch intToggle;
    [SerializeField] private Text valueReadout;

    public double min, max, skew;
    public bool wholeNumbers;

    private float value;

    public UnityEvent<string> onLabelChanged = new UnityEvent<string>();
    public UnityEvent<double> onMinChanged = new UnityEvent<double>();
    public UnityEvent<double> onMaxChanged = new UnityEvent<double>();
    public UnityEvent<double> onSkewChanged = new UnityEvent<double>();
    public UnityEvent<float> onSliderMoved = new UnityEvent<float>();

    private CircleSlider _circleSlider;

    protected override void ChildAwake()
    {
        _circleSlider = GetComponentInChildren<CircleSlider>();
        _circleSlider.onValueChanged.AddListener(val =>
        {
            var skewedVal = Mathf.Pow(val, (float)skew);
            value = (float)(min + ((max - min) * skewedVal));
            if (wholeNumbers)
            {
                value = (int)value;
            }
            valueReadout.text = Math.Round(value, 3).ToString();
            onSliderMoved.Invoke(value);
        });
        
        moduleType = ModuleType.KnobModule;

        minInput.onValueChanged.AddListener(input =>
        {
            if (!double.TryParse(input, out var val))
            {
                return;
            }
            min = val;
            RecalculateSliderPosition();
            onMinChanged.Invoke(min);
        });

        maxInput.onValueChanged.AddListener(input =>
        {
            if (!double.TryParse(input, out var val))
            {
                return;
            }
            max = val;
            RecalculateSliderPosition();
            onMaxChanged.Invoke(max);            
        });

        skewInput.onValueChanged.AddListener(input =>
        {
            if (!double.TryParse(input, out var val))
            {
                return;
            }
            skew = val;
            RecalculateSliderPosition();
            onSkewChanged.Invoke(skew);
        });

        intToggle.onValueChange.AddListener(state =>
        {
            wholeNumbers = state;
            RecalculateSliderPosition();
        });
    }

    public void InvokeSliderCallback()
    {
        _circleSlider.onValueChanged.Invoke(_circleSlider.Value);
    }

    private void RecalculateSliderPosition()
    {
        if (value < min)
        {
            value = (float)min;
        }

        if (wholeNumbers)
        {
            value = (int)value;
        }

        float normalised = (float)((value - min) / (max - min));
        var skewed = Mathf.Pow(normalised, 1.0f / (float)skew);
        _circleSlider.Set(skewed, true);
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

        value = (float)val;
        RecalculateSliderPosition();
    }

    public void SetValues(double mn, double mx, double val, double sk, bool wholeNums)
    {
        min = mn;
        minInput.SetText(min.ToString(), false);
        max = mx;
        maxInput.SetText(max.ToString(), false);
        skew = sk;
        skewInput.SetText(skew.ToString(), false);

        wholeNumbers = wholeNums;
        intToggle.Set(wholeNumbers, false);

        value = wholeNumbers ? (int)val : (float)val;
        RecalculateSliderPosition();
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
