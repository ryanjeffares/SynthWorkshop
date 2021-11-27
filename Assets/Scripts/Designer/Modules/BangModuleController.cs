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

    protected override void ChildAwake()
    {
        button.onClick.AddListener(() =>
        {
            buttonPressed.Invoke();
            StartCoroutine(image.LerpColour(Color.grey, 0.05f));
            StartCoroutine(image.LerpColour(Color.white, 0.05f, null, 0.2f));
        });
    }

    public override List<ModuleException> CheckErrors()
    {
        throw new System.NotImplementedException();
    }
}
