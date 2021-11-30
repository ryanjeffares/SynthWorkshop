using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BangDelayModuleController : ModuleParent
{
    public float Delay { get; private set; }

    public void Setup(float delay)
    {
        Delay = delay;
        nameText.text = $"delay {delay}";
    }    

    public override List<ModuleException> CheckErrors()
    {
        return new List<ModuleException>();
    }
}
