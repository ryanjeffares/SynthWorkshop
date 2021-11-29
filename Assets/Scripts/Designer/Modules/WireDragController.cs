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

    Vector3 _previousParentPos;

    private void Update()
    {
        var parentPos = parentController.transform.localPosition;
        parentPos.y += 15;
        parentPos = Vector3.Lerp(_previousParentPos, parentPos, Time.deltaTime);

        var diff = parentPos - transform.localPosition;
        _lineRenderer.Points[1] = diff;
        _lineRenderer.SetVerticesDirty();

        _previousParentPos = parentPos;

        if (!isConnected) return;

        transform.position = targetController.transform.position;

        // was connected but one of its connections has been destroyed, it is not needed anymore
        if (parentController == null || targetController == null)
        {
            Destroy(gameObject);
            return;
        }     
    }
}
