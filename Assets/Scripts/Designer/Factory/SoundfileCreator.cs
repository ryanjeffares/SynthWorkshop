using UnityEngine;

public class SoundfileCreator : IModuleCreator
{
    public GameObject CreateModule(GameObject prefab, Transform parent, Vector3 position, string[] input)
    {
        if (input.Length > 1)
        {
            return null;
        }

        return Object.Instantiate(prefab, position, Quaternion.identity, parent);
    }
}