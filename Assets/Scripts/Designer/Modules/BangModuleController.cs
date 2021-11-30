using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Events;

public class BangModuleController : ModuleParent
{
    [SerializeField] private Button button;
    [SerializeField] private Image image;

    public UnityEvent buttonPressed = new UnityEvent();

    private bool _state;

    protected override void ChildAwake()
    {
        button.onClick.AddListener(() =>
        {
            buttonPressed.Invoke();
            StartCoroutine(image.LerpColour(Color.grey, 0.05f));
            StartCoroutine(image.LerpColour(Color.white, 0.05f, null, 0.2f));
        });
    }

    private void Update()
    {
        var state = SynthWorkshopLibrary.GetTriggerableState(GlobalIndex);
        if (_state != state)
        {
            _state = state;
            StartCoroutine(image.LerpColour(_state ? Color.grey : Color.white, 0.05f));
        }
    }

    public override List<ModuleException> CheckErrors()
    {
        return new List<ModuleException>();
    }
}
