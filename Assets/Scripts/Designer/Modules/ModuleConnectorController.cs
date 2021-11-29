using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;
using UnityEngine.UI.Extensions;

public class ModuleConnectorController : MonoBehaviour, IPointerEnterHandler, IPointerExitHandler, 
    IDragHandler, IBeginDragHandler, IEndDragHandler, IPointerDownHandler
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

    private void DestroyWire(GameObject o)
    {
        _instantiatedWires.Remove(o);
        Destroy(o);
    }

    public void OnBeginDrag(PointerEventData eventData)
    {
        if (inputOutput == InputOutput.Input) return;

        _dragging = true;
        var wire = Instantiate(wirePrefab, transform);
        wire.GetComponent<WireDragController>().parentController = this;
        _instantiatedWires.Add(wire);
    }

    public void OnDrag(PointerEventData eventData)
    {
        if (inputOutput == InputOutput.Input) return;

        var wire = _instantiatedWires.Last();
        wire.transform.position = eventData.position;
    }

    public void OnEndDrag(PointerEventData eventData)
    {
        if (inputOutput == InputOutput.Input) return;

        parentModule.draggable = true;
        _dragging = false;
        
        var wire = _instantiatedWires.Last();
        var wireController = wire.GetComponent<WireDragController>();
        var target = wireController.targetController;

        var res = 
            (parentModule.moduleType == ModuleType.ToggleModule || parentModule.moduleType == ModuleType.BangModule)
            ? HandleTriggerConnection(target, wire, wireController)
            : HandleProcessorConnection(target, wire, wireController);

        if (res)
        {
            wireController.isConnected = true;
            isConnected = true;

            target.GetComponent<Image>().color = Color.green;
            GetComponent<Image>().color = Color.green;

            target.sourceConnector = this;
            target.connectedModuleConnectors.Add(this);
            connectedModuleConnectors.Add(target);
        }
    }

    // returns true if a condition is present that should prevent the wire from connecting
    private bool CheckTargetProcessorModules(ModuleConnectorController target)
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

    private bool CheckTargetTriggerModules(ModuleConnectorController target)
    {
        return (target == null)
            || (target.audioCv != AudioCV.Trigger || audioCv != AudioCV.Trigger)
            || (inputOutput == target.inputOutput)
            || connectedModuleConnectors.Contains(target)
            || target.connectedModuleConnectors.Contains(this);
    }

    private bool HandleProcessorConnection(ModuleConnectorController target, GameObject wire, WireDragController wireController)
    {        
        if (CheckTargetProcessorModules(target))
        {
            DestroyWire(wire);
            return false;
        }

        // need to know which of the target inputs we're connecting to
        // and the output_id of this connector
        var idx = target.parentModule.GetIndexOfConnector(target);
        var audioOutput = target.parentModule.moduleType == ModuleType.IOModule && target.audioCv == AudioCV.Audio;
        var outputIndex = audioCv == AudioCV.Audio ? _audioOutputIndex : _cvOutputIndex;

        if (!SynthWorkshopLibrary.SetModuleInputIndex(audioOutput, true, target.parentModule.GlobalIndex, outputIndex, idx))
        {
            DestroyWire(wire);
            return false;
        }        

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

        return true;
    }

    private bool HandleTriggerConnection(ModuleConnectorController target, GameObject wire, WireDragController wireController)
    {
        if (CheckTargetTriggerModules(target))
        {
            DestroyWire(wire);
            return false;
        }

        // so all we need to know here is the global id of the sender and target
        var senderId = parentModule.GlobalIndex;
        var targetId = target.parentModule.GlobalIndex;

        if (!SynthWorkshopLibrary.SetTriggerTarget(true, senderId, targetId))
        {
            DestroyWire(wire);
            return false;
        }

        return true;
    }

    private void ModuleDestroyedCallback(GameObject g)
    {
        var destroyedConnectors = g.GetComponentsInChildren<ModuleConnectorController>();

        foreach (var dc in destroyedConnectors.Where(d => connectedModuleConnectors.Contains(d)))
        {
            switch (parentModule.moduleType)
            {
                case ModuleType.ToggleModule:
                case ModuleType.BangModule:
                    SynthWorkshopLibrary.SetTriggerTarget(false, dc.parentModule.GlobalIndex, parentModule.GlobalIndex);
                    break;
                default:
                    var isAudio = parentModule.moduleType == ModuleType.IOModule;
                    SynthWorkshopLibrary.SetModuleInputIndex(isAudio, false, parentModule.GlobalIndex,
                        isAudio ? dc._audioOutputIndex : dc._cvOutputIndex, parentModule.GetIndexOfConnector(this));
                    break;
            }

            var wire = _instantiatedWires.FirstOrDefault(w => destroyedConnectors.Contains(w.GetComponent<WireDragController>().targetController));
            if (wire != null)
            {
                DestroyWire(wire);
            }

            connectedModuleConnectors.Remove(dc);
        }

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

        _firstClickReceived = false;
        _secondClickReceived = false;
    }

    public void RemoveConnection(ModuleConnectorController toRemove)
    {
        if (inputOutput == InputOutput.Output)
        {
            var wire = _instantiatedWires.Find(w => w.GetComponent<WireDragController>().targetController == toRemove);                

            if (wire == null) return;
            
            switch (parentModule.moduleType)
            {
                case ModuleType.BangModule:
                case ModuleType.ToggleModule:
                    SynthWorkshopLibrary.SetTriggerTarget(false, parentModule.GlobalIndex, toRemove.parentModule.GlobalIndex);
                    break;
                default:
                    var isAudio = toRemove.parentModule.moduleType == ModuleType.IOModule;
                    SynthWorkshopLibrary.SetModuleInputIndex(
                        isAudio,
                        false,
                        toRemove.parentModule.GlobalIndex,
                        audioCv == AudioCV.Audio ? _audioOutputIndex : _cvOutputIndex,
                        toRemove.parentModule.GetIndexOfConnector(toRemove)
                    );
                    break;
            }

            toRemove.SourceConnectionRemoved(this);
            _instantiatedWires.Remove(wire.gameObject);
            Destroy(wire);
        }
        else
        {
            var wire = toRemove._instantiatedWires.Find(w => w.GetComponent<WireDragController>().targetController == this);

            if (wire == null) return;

            switch (toRemove.parentModule.moduleType)
            {
                case ModuleType.BangModule:
                case ModuleType.ToggleModule:
                    SynthWorkshopLibrary.SetTriggerTarget(false, toRemove.parentModule.GlobalIndex, parentModule.GlobalIndex);
                    break;
                default:
                    var isAudio = parentModule.moduleType == ModuleType.IOModule;
                    var outIndex = audioCv == AudioCV.Audio ? toRemove._audioOutputIndex : toRemove._cvOutputIndex;
                    SynthWorkshopLibrary.SetModuleInputIndex(
                        isAudio,
                        false,
                        parentModule.GlobalIndex,
                        outIndex,
                        parentModule.GetIndexOfConnector(this)
                    );
                    break;
            }

            toRemove.RemoveWire(wire, this);
        }                       

        connectedModuleConnectors.Remove(toRemove);        
        if (connectedModuleConnectors.Count == 0)
        {
            GetComponent<Image>().color = Color.white;
        }        
    }

    private void SourceConnectionRemoved(ModuleConnectorController removed)
    {
        connectedModuleConnectors.Remove(removed);
        if (connectedModuleConnectors.Count == 0)
        {
            GetComponent<Image>().color = Color.white;
        }
    }

    private void RemoveWire(GameObject wire, ModuleConnectorController target)
    {
        connectedModuleConnectors.Remove(target);
        if (connectedModuleConnectors.Count == 0)
        {
            GetComponent<Image>().color = Color.white;
        }

        _instantiatedWires.Remove(wire);
        Destroy(wire);
    }
}

