using System.Collections;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using UnityEngine.Events;

public class ToggleSwitch : MonoBehaviour, IPointerClickHandler, IPointerEnterHandler, IPointerExitHandler
{
    [SerializeField] private Color offColour, onColour;
    [SerializeField] private Image background;
    [SerializeField] private GameObject indicatorIconParent;
    [SerializeField] private Image offIcon, onIcon;
    [SerializeField] private AnimationCurve curve;
    [SerializeField] private bool state;

    [SerializeField] private Color onIconColour = Color.white;
    [SerializeField] private Color offIconColour = new Color(1, 1, 1, 0);

    private readonly Color _hoveredColour = new Color(0.8f, 0.8f, 0.8f);

    public UnityEvent<bool> onValueChange = new UnityEvent<bool>();

    public bool IsOn
    {
        get => state;
    }

    private void Awake()
    {
        background.color = state ? onColour : offColour;
        indicatorIconParent.transform.localPosition =
            new Vector3(GetComponent<RectTransform>().sizeDelta.x / (state ? 4.0f : -4.0f), 0);
    }

    public void SetColours(Color on, Color off, bool interpolate)
    {
        offColour = off;
        onColour = on;
        if (interpolate)
        {
            StartCoroutine(background.LerpColour(state ? onColour : offColour, 0.3f, curve: curve));
        }
        else
        {
            background.color = state ? onColour : offColour;
        }
    }

    public void Set(bool incoming, bool invokeCallback)
    {
        state = !incoming;
        AnimateSwitch(invokeCallback);
    }
        
    public void AnimateSwitch(bool invokeCallback)
    {
        state = !state;

        if (invokeCallback)
        {
            onValueChange.Invoke(state);
        }

        ChangeVisualState();
    }

    private void ChangeVisualState()
    {
        if (gameObject.activeInHierarchy)
        {
            StartCoroutine(indicatorIconParent.transform.gameObject.InterpolatePosition(
                new Vector3(GetComponent<RectTransform>().sizeDelta.x / (state ? 4.0f : -4.0f), 0),
                0.3f,
                curve
            ));
            StartCoroutine(background.LerpColour(state ? onColour : offColour, 0.3f));
            StartCoroutine(offIcon.LerpColour(state ? offIconColour : onIconColour, 0.3f, curve: curve));
            StartCoroutine(onIcon.LerpColour(state ? onIconColour : offIconColour, 0.3f, curve: curve));
        }
        else
        {
            indicatorIconParent.transform.localPosition =
                new Vector3(GetComponent<RectTransform>().sizeDelta.x / (state ? 4.0f : -4.0f), 0);
            background.color = state ? onColour : offColour;
            offIcon.color = state ? offIconColour : onIconColour;
            onIcon.color = state ? onIconColour : offIconColour;
        }
    }

    public void OnPointerClick(PointerEventData eventData)
    {
        state = !state;

        onValueChange.Invoke(state);

        ChangeVisualState();
    }

    public void OnPointerEnter(PointerEventData eventData)
    {
        var targetImage = state ? onIcon : offIcon;
        StartCoroutine(targetImage.LerpColour(_hoveredColour, 0.1f));
    }

    public void OnPointerExit(PointerEventData eventData)
    {
        var targetImage = state ? onIcon : offIcon;
        StartCoroutine(targetImage.LerpColour(onIconColour, 0.1f));
    }
}
