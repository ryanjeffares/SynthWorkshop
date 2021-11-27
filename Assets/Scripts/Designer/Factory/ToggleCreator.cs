using UnityEngine;

public class ToggleCreator : IModuleCreator
{
    public GameObject CreateModule(GameObject prefab, Transform parent, Vector3 position, string[] input)
    {
        bool initialState;
        switch (input.Length)
        {
            case 1:
                initialState = false;
                break;
            case 2:
                if (!int.TryParse(input[1], out var val))
                {
                    return null;
                }

                initialState = val > 0;
                break;
            default: return null;
        }

        var obj = Object.Instantiate(prefab, position, Quaternion.identity, parent);
        obj.GetComponent<ToggleModuleController>().SetState(initialState, false);
        return obj;
    }
}