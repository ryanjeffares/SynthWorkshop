using UnityEngine;
using UnityEngine.UI;

public class ParamSliderController : MonoBehaviour
{
    [SerializeField] private Text nameText;

    public void Setup(string paramName, float min, float max, int targetIndex)
    {
        nameText.text = paramName;
        var s = GetComponent<Slider>();
        s.minValue = min;
        s.maxValue = max;
        s.onValueChanged.AddListener(val => SynthWorkshopLibrary.SetCvParam(targetIndex, val));
    }
}
