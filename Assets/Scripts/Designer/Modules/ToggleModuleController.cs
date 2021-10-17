using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Events;

public class ToggleModuleController : ModuleParent
{
    [SerializeField] private InputField labelInput;

    public UnityEvent<string> onLabelChanged = new UnityEvent<string>();
    public string Label { get; private set; }

    protected override void ChildAwake()
    {
        labelInput.onValueChanged.AddListener(val =>
        {
            Label = val;
            onLabelChanged.Invoke(Label);
        });
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
            var e = new ModuleException("Toggle's output is not connected and will be ignored.",
                ModuleException.SeverityLevel.Warning);
            exceptions.Add(e);
        }

        if (string.IsNullOrEmpty(Label))
        {
            var e = new ModuleException("Toggle has no label and will be difficult to identify.",
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
