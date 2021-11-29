using System;
using System.Collections;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Events;
using UnityEngine.EventSystems;
using TMPro;

[RequireComponent(typeof(TMP_InputField))]
public class TextInputField : MonoBehaviour, IPointerEnterHandler, IPointerExitHandler, IPointerDownHandler
{
    [SerializeField] private TextMeshProUGUI placeholder;
    [SerializeField] private Image background, horizontalLine;
    [SerializeField] private AnimationCurve curve;

    public UnityEvent<string> onValueChanged = new UnityEvent<string>();
    public UnityEvent<string> onSubmit = new UnityEvent<string>();
    public UnityEvent<string> onDeselect = new UnityEvent<string>();

    private TMP_InputField _inputField;

    private void Awake()
    {
        _inputField = GetComponent<TMP_InputField>();
        _inputField.onValueChanged.AddListener(InputValueChanged);
        _inputField.onSelect.AddListener(input => InputFieldSelected(input, true));
        _inputField.onDeselect.AddListener(input => InputFieldSelected(input, false));
        _inputField.onSubmit.AddListener(input => onSubmit.Invoke(input));
    }

    public string GetText()
    {
        _inputField ??= GetComponent<TMP_InputField>();            
        return _inputField.text;
    }

    public void SetText(string text, bool invokeCallback)
    {
        _inputField ??= GetComponent<TMP_InputField>();
        if (invokeCallback)
        {
            _inputField.text = text;
        }
        else
        {
            _inputField.SetTextWithoutNotify(text);
        }
    }

    public void SetInteractable(bool state)
    {
        _inputField ??= GetComponent<TMP_InputField>();
        _inputField.interactable = state;
    }

    public void SetTextColour(Color col)
    {
        foreach (var t in GetComponentsInChildren<TextMeshProUGUI>())
        {
            t.color = col;
        }
    }

    public void Select()
    {
        _inputField ??= GetComponent<TMP_InputField>();
        _inputField.Select();
    }

    private bool _firstClickReceived;
    private bool _secondClickReceived;

    public void OnPointerDown(PointerEventData eventData)
    {
        if (!_firstClickReceived)
        {
            _firstClickReceived = true;
            StartCoroutine(WaitForDoubleClick(eventData));
        }
        else
        {
            if (!_secondClickReceived)
            {
                _secondClickReceived = true;
            }
        }
    }

    private IEnumerator WaitForDoubleClick(PointerEventData eventData)
    {
        yield return new WaitForSeconds(0.2f);

        if (_secondClickReceived)
        {
            Destroy(gameObject);
        }

        _firstClickReceived = false;
        _secondClickReceived = false;
    }

    public void OnPointerEnter(PointerEventData eventData)
    {
        StartCoroutine(background.LerpColour(new Color(0.2f, 0.2f, 0.2f, 0.2f), 0.2f, curve: curve));
    }

    public void OnPointerExit(PointerEventData eventData)
    {
        StartCoroutine(background.LerpColour(new Color(0.7f, 0.7f, 0.7f, 0.2f), 0.2f, curve: curve));
    }

    private void InputValueChanged(string input)
    {
        onValueChanged.Invoke(input);
    }

    private void InputFieldSelected(string _, bool selected)
    {        
        StartCoroutine(FillLine(selected));
        StartCoroutine(placeholder.LerpColour(selected ? Color.clear : new Color(0.196f, 0.196f, 0.196f, 0.5f),
            0.2f));
    }

    private IEnumerator FillLine(bool fill)
    {
        const float time = 0.2f;
        float timeCounter = 0;

        while (timeCounter <= time)
        {
            var fillAmt = fill ? curve.Evaluate(timeCounter / time) : 1 - curve.Evaluate(timeCounter / time);
            horizontalLine.fillAmount = fillAmt;
            timeCounter += Time.deltaTime;
            yield return null;
        }

        horizontalLine.fillAmount = fill ? 1 : 0;
    }
}