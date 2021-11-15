using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class FilterModuleController : ModuleParent
{
    public float Cutoff { get; private set; } = 20000;
    public float Resonance { get; private set; } = 0.7f;
    public int FilterType { get; private set; } = 0;

    public void SetLabels(float hz, float q, int type)
    {
        Cutoff = hz;
        Resonance = q;
        FilterType = type;

        nameText.text = $"filt~ {hz} {q} {type}";
    }

    public void SetOutputIndex(int idx)
    {
        connectors[4].SetOutputIndex(idx);
    }

    public override List<ModuleException> CheckErrors()
    {
        throw new System.NotImplementedException();
    }
}