using System;
using System.Linq;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;

public class ModuleConnectorController : MonoBehaviour, IPointerEnterHandler, IPointerExitHandler, IDragHandler, IBeginDragHandler, IEndDragHandler
{
    [SerializeField] private GameObject wirePrefab;

    public AudioCV audioCv;
    public InputOutput inputOutput;
    public ModuleConnectorController sourceConnector;
    public bool isConnected;
    public List<ModuleConnectorController> connectedModuleConnectors;

    private ModuleParent _parent;
    private List<GameObject> _wires;
    private Vector3 _mousePosition;
    private bool _dragging;

    private void Awake()
    {
        ModuleParent.ModuleDestroyed += ModuleDestroyedCallback;
        _wires = new List<GameObject>();
        _parent = GetComponentInParent<ModuleParent>();
        connectedModuleConnectors = new List<ModuleConnectorController>();
    }

    private void OnDestroy()
    {
        ModuleParent.ModuleDestroyed -= ModuleDestroyedCallback;
        foreach (var w in _wires)
        {
            Destroy(w);
        }
    }

    private void ModuleDestroyedCallback(GameObject g)
    {
        var destroyedConnectors = g.GetComponentsInChildren<ModuleConnectorController>();
        foreach(var dc in destroyedConnectors)
        {
            if (connectedModuleConnectors.Contains(dc))
            {
                connectedModuleConnectors.Remove(dc);
            }
        }
    }

    public void SetTypes(AudioCV it, InputOutput i)
    {
        audioCv = it;
        inputOutput = i;
    }

    public void AddWire(GameObject wire)
    {
        _wires.Add(wire);
        isConnected = true;
    }
    
    public void UpdateWirePositions()
    {
        try
        {
            foreach (var w in _wires)
            {
                w.GetComponent<WireController>().UpdatePosition();
            }
        }
        catch
        {
            Debug.Log(transform.parent.name);
        }
    }

    public void OnPointerEnter(PointerEventData eventData)
    {
        _parent.draggable = false;
        StartCoroutine(gameObject.InterpolateSize(new Vector3(33, 33), 0.05f));
    }

    public void OnPointerExit(PointerEventData eventData)
    {
        if (!_dragging)
        {
            _parent.draggable = true;
        }        
        StartCoroutine(gameObject.InterpolateSize(new Vector3(30, 30), 0.05f));
    }

    public void OnBeginDrag(PointerEventData eventData)
    {
        _wires.Add(Instantiate(wirePrefab, transform));
        _mousePosition = eventData.position;
        _dragging = true;
    }

    public void OnDrag(PointerEventData eventData)
    {
        try
        {
            // yikes, this also throws a load of assertion failures lol
            var w = _wires.Last();
            var rect = w.GetComponent<RectTransform>();
            var up = eventData.position.y > _mousePosition.y;
            rect.pivot = new Vector2(0.5f, up ? 0 : 1);
            rect.sizeDelta = new Vector2(10, Vector3.Distance(eventData.position, _mousePosition));
            w.GetComponent<BoxCollider2D>().size = rect.sizeDelta;
            w.GetComponent<BoxCollider2D>().offset = new Vector2(0, rect.sizeDelta.y / (up ? 2 : -2));
            var opposite = eventData.position.y - _mousePosition.y;
            var adjacent = eventData.position.x - _mousePosition.x;
            if(opposite != 0 && adjacent != 0)
            {
                var angle = (Mathf.Atan(opposite / adjacent) * Mathf.Rad2Deg) - 90;
                // HAAAA this is absolute dog shit
                if (angle >= -180 && angle <= -90)
                {
                    angle += 180;
                }
                rect.eulerAngles = new Vector3(0, 0, angle);
            }            
        }
        catch(Exception e)
        {
            Debug.Log(e.Message);
        }
    }

    public void OnEndDrag(PointerEventData eventData)
    {
        _dragging = false;
        _parent.draggable = true;
        if (_wires.Last().GetComponent<WireController>().CheckTarget(this))
        {
            isConnected = true;
            GetComponent<Image>().color = Color.green;
        }
        else
        {
            Destroy(_wires.Last());
            _wires.Remove(_wires.Last());
        }
    }

    public void RemoveWire(GameObject wire)
    {
        _wires.Remove(wire);
        if (_wires.Count == 0)
        {
            GetComponent<Image>().color = Color.white;
            isConnected = false;
        }
        var wireController = wire.GetComponent<WireController>();
        if(connectedModuleConnectors.Contains(wireController.GetParent()))
        {
            connectedModuleConnectors.Remove(wireController.GetParent());
        }
        if (connectedModuleConnectors.Contains(wireController.GetTarget()))
        {
            connectedModuleConnectors.Remove(wireController.GetTarget());
        }
    }
}

