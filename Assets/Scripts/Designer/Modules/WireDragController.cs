using System;
using System.Collections;
using UnityEngine;
using UnityEngine.UI.Extensions;

public class WireDragController : MonoBehaviour
{
    [NonSerialized] public ModuleConnectorController parentController;
    [NonSerialized] public ModuleConnectorController targetController;
    [NonSerialized] public bool isConnected;

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
        if (!isConnected) return;

        // was connected but one of its connections has been destroyed, it is not needed anymore
        if (parentController == null || targetController == null)
        {
            Destroy(gameObject);
            return;
        }
        
        var diff = transform.position - targetController.transform.position;
        _lineRenderer.Points[1] = diff;
        _lineRenderer.SetAllDirty();
        
    }
}
