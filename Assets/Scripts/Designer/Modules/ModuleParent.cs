using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;

public enum ModuleType
{
    OscillatorModule, ControlModule, IOModule, MathsModule, NumberBox
}

public abstract class ModuleParent : MonoBehaviour, IDragHandler, IBeginDragHandler, IPointerClickHandler
{
    public class ModuleException
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
        ChildDestroyed();
    }

    protected virtual void ChildDestroyed() { }

    public virtual bool CheckIfUsed()
    {
        return connectors.Any(c => c.isConnected);
    }

    public abstract List<ModuleException> CheckErrors();

    public virtual List<ModuleConnectorController> GetUsedOutputs(out bool found)
    {
        var res = new List<ModuleConnectorController>();
        foreach (var con in connectors.Where(c => c.inputOutput == InputOutput.Output))
        {
            if (con.isConnected)
            {
                res.Add(con);
            }
        }
        found = res.Count > 0;
        return res;
    }

    public void OnPointerClick(PointerEventData eventData)
    {
        if (eventData.clickCount == 2)
        {
            ModuleDestroyed?.Invoke(gameObject);
            Destroy(gameObject);
        }
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
        var connectors = GetComponentsInChildren<ModuleConnectorController>();
        foreach(var c in connectors)
        {            
            c.UpdateWirePositions();
        }
    }
}

