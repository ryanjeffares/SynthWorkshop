using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using UnityEngine.Events;

[ExecuteAlways]
public class CircleSlider : MonoBehaviour, IPointerDownHandler,
    IPointerUpHandler, IDragHandler,
    IPointerEnterHandler, IPointerExitHandler
{
    [SerializeField] private float min, max;
    [SerializeField] private float value;
    [SerializeField] private bool wholeNumbers, useReadout, enlargeOnPointerEnter;
    [SerializeField] private GameObject indicatorPivot;
    [SerializeField] private string units;
    [SerializeField] private Image fillImage;

    public UnityEvent<float> onValueChanged = new UnityEvent<float>();
    public float Min => min;
    public float Max => max;
    public float Value => value;

    private Text _readout;
    private RectTransform _hitRectTransform;
    private bool _pointerDown;

    private float _angleOnPointerDown;
    private float _sliderAngle;

    public float SliderAngle
    {
        get => _sliderAngle;
        set
        {
            _sliderAngle = Mathf.Clamp(value, 0.0f, 360.0f);
            fillImage.fillAmount = SliderAngle / 360.0f;
            indicatorPivot.transform.localEulerAngles = new Vector3(180, 0, SliderAngle);
        }
    }

    private void Awake()
    {
        _readout = GetComponentInChildren<Text>();
    }

#if UNITY_EDITOR
    private void Update()
    {
        if (min > max)
        {
            Debug.LogWarning("Min should not be greater than max!");
            min = max - 1;
            return;
        }

        value = value.Limit(min, max);
        if (wholeNumbers)
        {
            value = (int)value;
        }

        SliderAngle = value.Map(min, max, 0, 360);
        if (useReadout)
        {
            _readout.SetTruncatedValueText(value, units);
        }
    }
#endif

    public void Set(float val, bool invokeCallback)
    {
        value = val;
        if (wholeNumbers)
        {
            value = (int) value;
        }

        SliderAngle = value.Map(min, max, 0, 360);
        if (useReadout)
        {
            _readout.SetTruncatedValueText(value, units);
        }

        if (invokeCallback)
        {
            onValueChanged.Invoke(value);
        }
    }

    public void SetRange(float mn, float mx, bool invokeCallback)
    {
        if (min > max)
        {
            Debug.LogWarning("Min should not be greater than max!");
            return;
        }

        min = mn;
        max = mx;
        if (value < min)
        {
            value = min;
            SliderAngle = 0;
            if (invokeCallback)
            {
                onValueChanged.Invoke(value);
            }
        }

        if (value > max)
        {
            value = max;
            SliderAngle = 360;
            if (invokeCallback)
            {
                onValueChanged.Invoke(value);
            }
        }

        if (wholeNumbers)
        {
            value = (int) value;
        }

        if (useReadout)
        {
            _readout.SetTruncatedValueText(value, units);
        }
    }

    private bool HasValueChanged()
    {
        return SliderAngle != _angleOnPointerDown;
    }

    public void OnPointerEnter(PointerEventData eventData)
    {
        if (!enlargeOnPointerEnter) return;
        StartCoroutine(indicatorPivot.transform.GetChild(0).gameObject.InterpolateScale(Vector3.one * 1.05f, 0.05f));
    }

    public void OnPointerExit(PointerEventData eventData)
    {
        if (!enlargeOnPointerEnter) return;
        StartCoroutine(indicatorPivot.transform.GetChild(0).gameObject.InterpolateScale(Vector3.one, 0.05f));
    }

    public void OnPointerDown(PointerEventData eventData)
    {
        _hitRectTransform = eventData.pointerCurrentRaycast.gameObject.GetComponent<RectTransform>();
        _pointerDown = true;
        _angleOnPointerDown = SliderAngle;
        HandleInput(eventData);
    }

    public void OnPointerUp(PointerEventData eventData)
    {
        _hitRectTransform = null;
        _pointerDown = false;
    }

    public void OnDrag(PointerEventData eventData)
    {
        HandleInput(eventData);
    }

    private void HandleInput(PointerEventData eventData)
    {
        if (!_pointerDown) return;

        RectTransformUtility.ScreenPointToLocalPointInRectangle(_hitRectTransform, eventData.position,
            eventData.pressEventCamera, out var localPos);

        var newAngle = Mathf.Atan2(-localPos.y, localPos.x) * Mathf.Rad2Deg + 180f;
        var currentAngle = SliderAngle;

        if (Mathf.Abs(newAngle - currentAngle) >= 180)
        {
            newAngle = currentAngle < newAngle ? 0.0f : 360.0f;
        }

        SliderAngle = newAngle;

        if (HasValueChanged())
        {
            value = SliderAngle.Map(0, 360, min, max);
            if (wholeNumbers)
            {
                value = (int) value;
            }

            if (useReadout)
            {
                _readout.SetTruncatedValueText(value, units);
            }

            onValueChanged.Invoke(value);
        }
    }
}