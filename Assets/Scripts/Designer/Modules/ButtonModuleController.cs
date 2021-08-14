using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class ButtonModuleController : ModuleParent
{
    [SerializeField] private InputField labelInput;

    public string Label { get; private set; }

    protected override void ChildAwake()
    {
        labelInput.onValueChanged.AddListener(val => Label = val);
    }

    public void SetLabel(string label)
    {
        Label = label;
        labelInput.text = Label;
    }
    
    public override List<ModuleException> CheckErrors()
    {
        var exceptions = new List<ModuleException>();
        if (!connectors[0].isConnected)
        {
            var e = new ModuleException("Button's output is not connected and will be ignored.",
                ModuleException.SeverityLevel.Warning);
            exceptions.Add(e);
        }
        if (string.IsNullOrEmpty(labelInput.text))
        {
            var e = new ModuleException("Button has no label, and will be hard to identify.",
                ModuleException.SeverityLevel.Warning);
            exceptions.Add(e);
        }
        return exceptions;
    }
    
    public int GetOutputIdOfConnector(Dictionary<ModuleConnectorController, int> lookup)
    {
        return lookup[connectors[0]];
    }
}
