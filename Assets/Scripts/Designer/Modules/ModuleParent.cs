using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;

public enum ModuleType
{
    OscillatorModule, KnobModule, IOModule, MathsModule, NumberBox, ADSR, ButtonModule, ToggleModule, FilterModule, BangModule, BangDelayModule
}

public enum AudioCV
{
    Audio, CV, Either, Trigger
}

public enum InputOutput
{
    Input, Output
}

public abstract class ModuleParent : MonoBehaviour, IDragHandler, IPointerDownHandler
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

    public ModuleType moduleType;
    public bool draggable = true;
    public bool isTriggerable;
    
    public string IdentifierName { get; private set; }
    public int GlobalIndex { get; private set; }

    public string DisplayName => nameText == null ? moduleType.ToString() : nameText.text;

    private void Awake()
    {
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

    public void OnPointerDown(PointerEventData eventData)
    {
        if (!_firstClickReceived)
        {
            _firstClickReceived = true;
            StartCoroutine(WaitForDoubleClick());
        }
        else
        {
            if (!_secondClickReceived)
            {
                _secondClickReceived = true;
            }
        }
    }

    private IEnumerator WaitForDoubleClick()
    {
        yield return new WaitForSeconds(0.2f);

        if (_secondClickReceived)
        {
            int moduleTypeInt = moduleType switch
            {
                ModuleType.IOModule => 0,
                ModuleType.ToggleModule => 2,
                ModuleType.BangModule => 2,
                _ => 1
            };
            SynthWorkshopLibrary.ModuleDestroyed(moduleTypeInt, GlobalIndex);
            Destroy(gameObject);
        }

        _firstClickReceived = false;
        _secondClickReceived = false;
    }

    public virtual void OnDrag(PointerEventData eventData)
    {
        if (!draggable) return;
        transform.position = eventData.position;
    }

    public int GetIndexOfConnector(ModuleConnectorController controller)
    {
        return connectors.IndexOf(controller);
    }
}

