using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

public class WireController : MonoBehaviour, IPointerClickHandler
{
    private GameObject _targetConnector;
    private GameObject _parentConnector;
    private bool _connected;

    private AudioCV _wireType;
    private AudioCV WireType
    {
        get => _wireType;
        set
        {
            _wireType = value;
            GetComponent<Image>().color = WireType == AudioCV.Audio ? Color.black : Color.grey;
        }
    }

    private void OnDestroy()
    {
        if (_targetConnector != null)
        {
            _targetConnector.GetComponent<ModuleConnectorController>().RemoveWire(gameObject);
        }
        if (_parentConnector != null)
        {
            _parentConnector.GetComponent<ModuleConnectorController>().RemoveWire(gameObject);
        }
    }

    private void OnTriggerEnter2D(Collider2D other)
    {
        if (_connected) return;
        _targetConnector = other.gameObject;
    }

    private void OnTriggerExit2D(Collider2D other)
    {
        if (_connected) return;
        _targetConnector = null;
    }

    public void OnPointerClick(PointerEventData eventData)
    {
        if (eventData.clickCount == 2)
        {
            Destroy(gameObject);
        }
    }

    public void UpdatePosition()
    {
        var rect = GetComponent<RectTransform>();
        var col = GetComponent<BoxCollider2D>();
        var parentPos = _parentConnector.transform.position;
        var targetPos = _targetConnector.transform.position;
        var up = targetPos.y > parentPos.y;
        rect.pivot = new Vector2(0.5f, up ? 0 : 1);
        rect.sizeDelta = new Vector2(10, Vector2.Distance(parentPos, targetPos));
        col.size = rect.sizeDelta;
        col.offset = new Vector2(0, rect.sizeDelta.y / (up ? 2 : -2));
        var opposite = parentPos.y - targetPos.y;
        var adjacent = parentPos.x - targetPos.x;
        var angle = (Mathf.Atan(opposite / adjacent) * Mathf.Rad2Deg) - 90;
        if (angle >= -180 && angle <= -90)
        {
            angle += 180;
        }
        rect.eulerAngles = new Vector3(0, 0, angle);
    }

    public bool CheckTarget(ModuleConnectorController parent)
    {
        // _targetConnector will be null if the wire is not dragged onto anything at all, so return false and delete the wire
        if (_targetConnector == null) return false;

        var targetController = _targetConnector.GetComponent<ModuleConnectorController>();

        // do we want to stop you from connecting a module to itself?
        //if (_targetConnector.GetComponentInParent<ModuleParent>() == parent.gameObject.GetComponentInParent<ModuleParent>()) return false;
        // return false if the audio/cv types or input/output do not match
        // return false if there is already a wire going between the same two connectors
        
        if (targetController.audioCv != AudioCV.Either && parent.audioCv != targetController.audioCv) return false;
        if (parent.inputOutput == targetController.inputOutput) return false;
        if (parent.connectedModuleConnectors.Contains(targetController) 
            || targetController.connectedModuleConnectors.Contains(parent)) return false;

        // TODO: return false if its a CV input that already has a connection - we should allow this at some point but figure that out later
        if (targetController.audioCv != AudioCV.Audio && targetController.isConnected) return false;

        WireType = parent.audioCv;
        _parentConnector = parent.gameObject;
        _targetConnector.GetComponent<Image>().color = Color.green;
        targetController.AddWire(gameObject);
        targetController.sourceConnector = parent;
        targetController.connectedModuleConnectors.Add(parent);
        parent.connectedModuleConnectors.Add(targetController);
        _connected = true;

        if(_targetConnector.transform.parent.parent.TryGetComponent(out MathsModuleController mathsController))
        {
            mathsController.SetAudioOrCvIncoming(parent.audioCv, targetController);
        }

        return true;
    }

    public ModuleConnectorController GetParent()
    {
        return _parentConnector == null ? null : _parentConnector.GetComponent<ModuleConnectorController>();
    }

    public ModuleConnectorController GetTarget()
    {
        return _targetConnector == null ? null : _targetConnector.GetComponent<ModuleConnectorController>();
    }
}
