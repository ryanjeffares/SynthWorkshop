using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;
using UnityEngine.UI.Extensions;

public class ModuleConnectorDrag : MonoBehaviour, IDragHandler, IEndDragHandler, IBeginDragHandler
{
    [SerializeField] private ModuleConnectorController parentConnector;
    [SerializeField] private Texture2D altCursor;
    
    private UILineRenderer _lineRenderer;
    private ModuleConnectorController _targetConnector;

    private bool _isConnected;
    private bool _isDragging;

    private void Awake()
    {
        _lineRenderer = GetComponent<UILineRenderer>();
    }

    public void OnBeginDrag(PointerEventData eventData)
    {
        _isDragging = true;
    }
    
    public void OnDrag(PointerEventData eventData)
    {
        transform.position = eventData.position;
    }

    public void OnEndDrag(PointerEventData eventData)
    {
        if (CheckTarget())
        {
            Debug.Log("False");
            transform.localPosition = Vector3.zero;
            return;
        }

        _targetConnector.GetComponent<Image>().color = Color.green;
        parentConnector.GetComponent<Image>().color = Color.green;

        _targetConnector.sourceConnector = parentConnector;
        _targetConnector.connectedModuleConnectors.Add(parentConnector);
        parentConnector.connectedModuleConnectors.Add(_targetConnector);

        if (_targetConnector.transform.parent.parent.TryGetComponent(out MathsModuleController mathsController))
        {
            mathsController.SetAudioOrCvIncoming(parentConnector.audioCv, _targetConnector);
        }

        _isConnected = true;
        _isDragging = false;
        parentConnector.parentModule.draggable = true;
    }

    // returns true if a condition is present that should prevent us from connecting
    private bool CheckTarget()
    {
        return _targetConnector == null
               || (_targetConnector.audioCv != AudioCV.Either && parentConnector.audioCv != _targetConnector.audioCv)
               || (parentConnector.inputOutput == _targetConnector.inputOutput)
               || parentConnector.connectedModuleConnectors.Contains(_targetConnector) 
               || _targetConnector.connectedModuleConnectors.Contains(parentConnector);
    }

    private void OnTriggerEnter2D(Collider2D other)
    {
        if (!other.gameObject.TryGetComponent<ModuleConnectorController>(out var target)) return;
    
        if (target == parentConnector) return;
        
        _targetConnector = target;
        Cursor.SetCursor(altCursor, Vector2.zero, CursorMode.ForceSoftware);
    }

    private void OnTriggerExit2D(Collider2D other)
    {
        if (!other.gameObject.TryGetComponent<ModuleConnectorController>(out var target)) return;

        if (target == parentConnector) return;
        
        _targetConnector = null;
        Cursor.SetCursor(null, Vector2.zero, CursorMode.ForceSoftware);
    }

    public void Update()
    {
        if (!_isDragging)
        {
            if (_isConnected)
            {
                transform.position = _targetConnector.transform.position;
            }
            else
            {
                transform.localPosition = Vector3.zero;
            }
        }   
        
        var diff = transform.position - parentConnector.transform.position;
        _lineRenderer.Points[0] = diff;
        _lineRenderer.SetAllDirty();
    }
}
