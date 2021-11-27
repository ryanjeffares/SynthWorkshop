using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Events;

public class ToggleModuleController : ModuleParent
{
    [SerializeField] private ToggleSwitch toggleSwitch;

    public UnityEvent<bool> toggleChanged = new UnityEvent<bool>();

    protected override void ChildAwake()
    {
        toggleSwitch.onValueChange.AddListener(state =>
        {
            toggleChanged.Invoke(state);
        });
    }

    public void SetState(bool state, bool sendCallback)
    {
        toggleSwitch.Set(state, sendCallback);
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

        return exceptions;
    }

    public int GetOutputIdOfConnector(Dictionary<ModuleConnectorController, int> lookup)
    {
        return lookup[connectors[0]];
    }
}
