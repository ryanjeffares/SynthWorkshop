using System.Collections;
using UnityEngine;
using UnityEngine.UI;

public class ParamButtonController : MonoBehaviour
{
    [SerializeField] private Text nameText;

    public void Setup(string name, int target)
    {
        nameText.text = name;
        GetComponent<Button>().onClick.AddListener(() => StartCoroutine(SetParam(target)));
    }

    private IEnumerator SetParam(int index)
    {
        SynthWorkshopLibrary.SetCvParam(index, 1);
        yield return new WaitForEndOfFrame();
        SynthWorkshopLibrary.SetCvParam(index, 0);
    }
}
