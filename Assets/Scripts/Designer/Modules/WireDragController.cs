using System;
using System.Collections;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI.Extensions;

public class WireDragController : MonoBehaviour
{
    [NonSerialized] public ModuleConnectorController parentController;
    [NonSerialized] public ModuleConnectorController targetController;
    [NonSerialized] public bool isConnected;

    private AudioCV _wireType;
    public AudioCV WireType
    {
        get => _wireType;
        set
        {
            _wireType = value;
            _lineRenderer.color = WireType switch
            {
                AudioCV.Audio => Color.black,
                AudioCV.CV => Color.grey,
                AudioCV.Trigger => Color.blue,
                _ => Color.black
            };
        }
    }

    private UILineRenderer _lineRenderer;

    private void Awake()
    {
        _lineRenderer = GetComponent<UILineRenderer>();
    }

    private void OnTriggerEnter2D(Collider2D other)
    {
        if (isConnected) return;
        if (other.TryGetComponent(out ModuleConnectorController target))
        {
            targetController = target;
        }
    }

    private void OnTriggerExit2D(Collider2D other)
    {
        if (isConnected) return;
        if (other.TryGetComponent(out ModuleConnectorController _))
        {
            targetController = null;
        }
    }

    private void Update()
    {
        var parentPos = parentController.transform.localPosition;
        parentPos.y += 15;

        var diff = parentPos - transform.localPosition;
        _lineRenderer.Points[1] = diff;
        _lineRenderer.SetVerticesDirty();

        if (!isConnected) return;

        transform.position = targetController.transform.position;
    }
}
