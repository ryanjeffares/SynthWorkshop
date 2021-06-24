using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class ParamToggleController : MonoBehaviour
{
    [SerializeField] private Text nameText;

    public void Setup(string name, int target)
    {
        nameText.text = name;
        GetComponent<Toggle>().onValueChanged.AddListener(val => SynthWorkshopLibrary.SetCvParam(target, val ? 1 : 0));
    }
}
