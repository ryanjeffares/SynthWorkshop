using UnityEngine;

public class BangCreator : IModuleCreator
{
    public GameObject CreateModule(GameObject prefab, Transform parent, Vector3 position, string[] input)
    {
        if (input.Length > 1)
        {
            return null;
        }

        var obj = Object.Instantiate(prefab, position, Quaternion.identity, parent);
        return obj;
    }
}
