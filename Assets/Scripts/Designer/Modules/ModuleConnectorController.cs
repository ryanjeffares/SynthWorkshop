using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;
using UnityEngine.UI.Extensions;

public class ModuleConnectorController : MonoBehaviour, IPointerEnterHandler, IPointerExitHandler, 
    IDragHandler, IBeginDragHandler, IEndDragHandler, IPointerClickHandler
{
    [SerializeField] private GameObject wirePrefab;
    [SerializeField] private GameObject connectionDisplayPrefab;
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

    private bool _pointerDown, _pointerEntered;
    private GameObject _connectionDisplay;

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
        var idx = target.parentModule.GetIndexOfConnector(target);
        var audioOutput = target.parentModule.moduleType == ModuleType.IOModule && target.audioCv == AudioCV.Audio;
        var outputIndex = audioCv == AudioCV.Audio ? _audioOutputIndex : _cvOutputIndex;

        var res = SynthWorkshopLibrary.SetModuleInputIndex(audioOutput, true, target.parentModule.GlobalIndex, outputIndex, idx);
        if (!res)
        {
            DestroyWire(wire);
            return;
        }

        target.GetComponent<Image>().color = Color.green;
        GetComponent<Image>().color = Color.green;

        target.sourceConnector = this;
        target.connectedModuleConnectors.Add(this);
        connectedModuleConnectors.Add(target);

        var targetParent = target.transform.parent.parent;

        if (targetParent.TryGetComponent(out MathsModuleController mathsController))
        {
            mathsController.SetAudioOrCvIncoming(audioCv, target);
        }
        else if (targetParent.TryGetComponent(out NumberModule numberModule))
        {
            numberModule.SetInputIndex(outputIndex);
        }

        if (parentModule is KnobModuleController knob)
        {
            knob.InvokeSliderCallback();
        }

        if (_connectionDisplay != null)
        {
            _connectionDisplay.GetComponent<ConnectionDisplayController>().AddConnection(target);
        }

        wireController.isConnected = true;
        isConnected = true;
    }

    // returns true if a condition is present that should prevent the wire from connecting
    private bool CheckTarget(ModuleConnectorController target)
    {
        bool firstCheck = target == null
               || (target.audioCv != AudioCV.Either && audioCv != target.audioCv)
               || (inputOutput == target.inputOutput)
               || connectedModuleConnectors.Contains(target)
               || target.connectedModuleConnectors.Contains(this);

        if (firstCheck)
        { 
            return true; 
        }

        if (target.parentModule is MathsModuleController m)
        {
            if (m.audioCv == AudioCV.Audio && m.IncomingSignalType != AudioCV.Either)
            {
                return m.IncomingSignalType != audioCv;
            }
        }

        return false;
    }

    private void ModuleDestroyedCallback(GameObject g)
    {
        var destroyedConnectors = g.GetComponentsInChildren<ModuleConnectorController>();

        var anyConnected = false;
        foreach (var dc in destroyedConnectors)
        {
            if (!connectedModuleConnectors.Contains(dc)) continue;
            
            anyConnected = true;

            var isAudio = parentModule.moduleType == ModuleType.IOModule;

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

    public void OnPointerEnter(PointerEventData eventData)
    {
        _pointerEntered = true;

        if (inputOutput == InputOutput.Input) return;

        Cursor.SetCursor(altCursor, Vector2.zero, CursorMode.ForceSoftware);
        parentModule.draggable = false;
    }

    public void OnPointerExit(PointerEventData eventData)
    {
        _pointerEntered = false;

        if (inputOutput == InputOutput.Input) return;
        Cursor.SetCursor(null, Vector2.zero, CursorMode.ForceSoftware);

        if (!_dragging)
        {
            parentModule.draggable = true;
        }
    }
    
    public void OnPointerClick(PointerEventData eventData)
    {
        if (eventData.clickCount == 2)
        {
            if (_connectionDisplay == null)
            {
                _connectionDisplay = Instantiate(connectionDisplayPrefab, transform);
                _connectionDisplay.GetComponent<ConnectionDisplayController>().Setup(this);
            }
            else
            {
                Destroy(_connectionDisplay);
            }
        }
    }

    public void RemoveConnection(ModuleConnectorController toRemove)
    {
        var wire = _instantiatedWires.Find(w => w.GetComponent<WireDragController>().targetController == toRemove);
        if (wire == null) return;

        var isAudio = parentModule.moduleType == ModuleType.IOModule;

        SynthWorkshopLibrary.SetModuleInputIndex(
            isAudio, 
            false, 
            toRemove.parentModule.GlobalIndex, 
            audioCv == AudioCV.Audio ? _audioOutputIndex : _cvOutputIndex, 
            toRemove.parentModule.GetIndexOfConnector(toRemove)
        );

        _instantiatedWires.Remove(wire);
        Destroy(wire);

        connectedModuleConnectors.Remove(toRemove);        
        if (connectedModuleConnectors.Count == 0)
        {
            GetComponent<Image>().color = Color.white;
        }

        toRemove.SourceConnectionRemoved(this);
    }

    private void SourceConnectionRemoved(ModuleConnectorController removed)
    {
        connectedModuleConnectors.Remove(removed);
        if (connectedModuleConnectors.Count == 0)
        {
            GetComponent<Image>().color = Color.white;
        }
    }
}

