﻿using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;

public enum ModuleType
{
    OscillatorModule, KnobModule, IOModule, MathsModule, NumberBox, ADSR, ButtonModule, ToggleModule, FilterModule, BangModule
}

public enum AudioCV
{
    Audio, CV, Either, Trigger
}

public enum InputOutput
{
    Input, Output
}

public abstract class ModuleParent : MonoBehaviour, IDragHandler, IBeginDragHandler, IPointerClickHandler
{
    public readonly struct ModuleException
    {
        public enum SeverityLevel
        {
            Warning, Error
        }

        public readonly string message;
        public readonly SeverityLevel severityLevel;

        public ModuleException(string m, SeverityLevel s)
        {
            message = m;
            severityLevel = s;
        }
    }

    public static Action<GameObject> ModuleDestroyed;
    
    [SerializeField] protected Text nameText;
    [SerializeField] protected List<ModuleConnectorController> connectors;

    protected Vector3 mousePosition, startLocalPos;
    protected float maxHorizontal, maxVertical;

    public ModuleType moduleType;
    public bool draggable = true;
    
    public string IdentifierName { get; private set; }
    public int GlobalIndex { get; private set; }

    public string DisplayName => nameText == null ? moduleType.ToString() : nameText.text;

    private void Awake()
    {
        var rect = transform.parent.GetComponent<RectTransform>().rect;
        maxHorizontal = rect.width / 2;
        maxVertical = rect.height / 2;
        ChildAwake();
    }

    protected virtual void ChildAwake() { }

    private void OnDestroy()
    {
        ModuleDestroyed?.Invoke(gameObject);        
        ChildDestroyed();
    }

    protected virtual void ChildDestroyed() { }

    public virtual bool CheckIfUsed()
    {
        return connectors.Any(c => c.isConnected);
    }

    public void SetIdentifier(string id, int idx)
    {
        IdentifierName = id;
        GlobalIndex = idx;
    }

    public abstract List<ModuleException> CheckErrors();

    public virtual List<ModuleConnectorController> GetUsedOutputs(out bool found)
    {
        var res = new List<ModuleConnectorController>();
        res.AddRange(connectors.Where(c => c.inputOutput == InputOutput.Output).Where(con => con.isConnected));
        found = res.Count > 0;
        return res;
    }

    private bool _firstClickReceived;
    private bool _secondClickReceived;

    public void OnPointerClick(PointerEventData eventData)
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

        int moduleTypeInt = moduleType switch
        {
            ModuleType.IOModule => 0,
            ModuleType.ToggleModule => 2,
            ModuleType.BangModule => 2,
            _ => 1
        };
        SynthWorkshopLibrary.ModuleDestroyed(moduleTypeInt, GlobalIndex);
        Destroy(gameObject);

        _firstClickReceived = false;
        _secondClickReceived = false;
    }

    public virtual void OnBeginDrag(PointerEventData eventData)
    {
        if (!draggable) return;
        mousePosition = eventData.position;
        startLocalPos = transform.localPosition;
    }

    public virtual void OnDrag(PointerEventData eventData)
    {
        if (!draggable) return;
        
        var x = eventData.position.x - mousePosition.x;
        var y = eventData.position.y - mousePosition.y;
        var newPos = transform.localPosition;
        
        if (startLocalPos.x + x < maxHorizontal && startLocalPos.x + x > -maxHorizontal)
        {
            newPos.x = startLocalPos.x + x;
        }
        if (startLocalPos.y + y < maxVertical && startLocalPos.y + y > -maxVertical)
        {
            newPos.y = startLocalPos.y + y;
        }
        
        transform.localPosition = newPos;
    }

    public int GetIndexOfConnector(ModuleConnectorController controller)
    {
        return connectors.IndexOf(controller);
    }
}

