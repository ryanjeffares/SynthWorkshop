using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class ConnectionDisplayController : MonoBehaviour
{
    [SerializeField] private GameObject nameDisplayPrefab;

    private ModuleConnectorController _parent;
    private readonly List<GameObject> _buttons = new List<GameObject>();

    public void Setup(ModuleConnectorController parent)
    {
        _parent = parent;

        foreach (var connection in parent.connectedModuleConnectors)
        {
            AddConnection(connection);
        }
    }

    public void AddConnection(ModuleConnectorController controller)
    {
        var obj = Instantiate(nameDisplayPrefab, transform.GetChild(0).GetChild(0));
        _buttons.Add(obj);

        obj.GetComponentInChildren<Text>().text = controller.parentModule.DisplayName;
        obj.GetComponent<Button>().onClick.AddListener(() =>
        {
            _parent.RemoveConnection(controller);
            _buttons.Remove(obj);            
            Destroy(obj);

            if (_buttons.Count == 0)
            {
                Destroy(gameObject);
            }
        });
    }
}
