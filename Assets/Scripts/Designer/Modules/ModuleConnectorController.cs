using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;
using UnityEngine.UI.Extensions;

public class ModuleConnectorController : MonoBehaviour, IPointerEnterHandler, IPointerExitHandler, IDragHandler, IBeginDragHandler, IEndDragHandler
{
    [SerializeField] private GameObject wirePrefab;
    [SerializeField] private Texture2D altCursor;

    public AudioCV audioCv;
    public InputOutput inputOutput;
    
    [NonSerialized]
    public bool isConnected;
    [NonSerialized]
    public ModuleConnectorController sourceConnector;
    [NonSerialized]
    public List<ModuleConnectorController> connectedModuleConnectors;

    [NonSerialized]
    public ModuleParent parentModule;

    private readonly List<GameObject> _instantiatedWires = new List<GameObject>();
    private bool _dragging;

    private int _cvOutputIndex, _audioOutputIndex;

    private void Awake()
    {
        ModuleParent.ModuleDestroyed += ModuleDestroyedCallback;
        parentModule = GetComponentInParent<ModuleParent>();
        connectedModuleConnectors = new List<ModuleConnectorController>();
    }

    private void OnDestroy()
    {
        Cursor.SetCursor(null, Vector2.zero, CursorMode.ForceSoftware);
        ModuleParent.ModuleDestroyed -= ModuleDestroyedCallback;
    }

    public void SetOutputIndex(int idx)
    {
        if (audioCv == AudioCV.Audio)
        {
            _audioOutputIndex = idx;
        }
        else if (audioCv == AudioCV.CV)
        {
            _cvOutputIndex = idx;
        }
    }

    public void OnBeginDrag(PointerEventData eventData)
    {
        _dragging = true;
        var wire = Instantiate(wirePrefab, transform);
        wire.GetComponent<WireDragController>().parentController = this;
        _instantiatedWires.Add(wire);
    }

    public void OnDrag(PointerEventData eventData)
    {
        var wire = _instantiatedWires.Last();
        wire.transform.position = eventData.position;

        var diff = wire.transform.position - transform.position;
        wire.GetComponent<UILineRenderer>().Points[0] = diff;
        wire.GetComponent<UILineRenderer>().SetAllDirty();
    }

    public void OnEndDrag(PointerEventData eventData)
    {
        void DestroyWire(GameObject o)
        {
            _instantiatedWires.Remove(o);
            Destroy(o);
        }

        parentModule.draggable = true;
        _dragging = false;
        
        var wire = _instantiatedWires.Last();
        var wireController = wire.GetComponent<WireDragController>();
        if (CheckTarget(wireController.targetController))
        {
            DestroyWire(wire);
            return;
        }
        
        var target = wireController.targetController;
        
        // need to know which of the target inputs we're connecting to
        // and the output_id of this connector
        switch (target.parentModule.moduleType)
        {
            case ModuleType.OscillatorModule:
            {
                var idx = target.parentModule.GetIndexOfConnector(target);
                var res = SynthWorkshopLibrary.SetModuleInputIndex(false, true, target.parentModule.GlobalIndex,
                    _cvOutputIndex, idx);
                if (!res)
                {
                    DestroyWire(wire);
                    return;
                }
                
                break;
            }
            case ModuleType.IOModule:
            {
                var idx = target.parentModule.GetIndexOfConnector(target);
                var res = SynthWorkshopLibrary.SetModuleInputIndex(true, true, target.parentModule.GlobalIndex,
                    _audioOutputIndex, idx);
                if (!res)
                {
                    DestroyWire(wire);
                    return;
                }

                break;
            }
        }

        target.GetComponent<Image>().color = Color.green;
        GetComponent<Image>().color = Color.green;

        target.sourceConnector = this;
        target.connectedModuleConnectors.Add(this);
        connectedModuleConnectors.Add(target);

        if (target.transform.parent.parent.TryGetComponent(out MathsModuleController mathsController))
        {
            mathsController.SetAudioOrCvIncoming(audioCv, target);
        }

        if (parentModule is KnobModuleController knob)
        {
            knob.InvokeSliderCallback();
        }

        wireController.isConnected = true;
        isConnected = true;
    }

    // returns true if a condition is present that should prevent the wire from connecting
    private bool CheckTarget(ModuleConnectorController target)
    {
        return target == null
               || (target.audioCv != AudioCV.Either && audioCv != target.audioCv)
               || (inputOutput == target.inputOutput)
               || connectedModuleConnectors.Contains(target)
               || target.connectedModuleConnectors.Contains(this);
    }

    private void ModuleDestroyedCallback(GameObject g)
    {
        var destroyedConnectors = g.GetComponentsInChildren<ModuleConnectorController>();

        var anyConnected = false;
        foreach (var dc in destroyedConnectors)
        {
            if (!connectedModuleConnectors.Contains(dc)) continue;
            
            anyConnected = true;

            var isAudio = audioCv == AudioCV.Audio;
            SynthWorkshopLibrary.SetModuleInputIndex(isAudio, false, parentModule.GlobalIndex,
                isAudio ? dc._audioOutputIndex : dc._cvOutputIndex, parentModule.GetIndexOfConnector(this));
                
            connectedModuleConnectors.Remove(dc);
        }

        if (!anyConnected) return;
        
        _instantiatedWires.Clear();

        if (connectedModuleConnectors.Count == 0)
        {
            GetComponent<Image>().color = Color.white;
        }
    }

    public void SetTypes(AudioCV it, InputOutput i)
    {
        audioCv = it;
        inputOutput = i;
    }

    public void AddWire(GameObject wire)
    {
        isConnected = true;
    }

    public void OnPointerEnter(PointerEventData eventData)
    {
        if (inputOutput == InputOutput.Input) return;
        Cursor.SetCursor(altCursor, Vector2.zero, CursorMode.ForceSoftware);
        parentModule.draggable = false;
    }

    public void OnPointerExit(PointerEventData eventData)
    {
        if (inputOutput == InputOutput.Input) return;
        Cursor.SetCursor(null, Vector2.zero, CursorMode.ForceSoftware);

        if (!_dragging)
        {
            parentModule.draggable = true;
        }
    }
}

