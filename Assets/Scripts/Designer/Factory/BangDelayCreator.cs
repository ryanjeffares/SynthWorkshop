using UnityEngine;

public class BangDelayCreator : IModuleCreator
{
    public GameObject CreateModule(GameObject prefab, Transform parent, Vector3 position, string[] input)
    {
        if (input.Length != 2)
        {
            return null;
        }

        if (!float.TryParse(input[1], out var delay))
        {
            return null;
        }

        var obj = Object.Instantiate(prefab, position, Quaternion.identity, parent);
        obj.GetComponent<BangDelayModuleController>().Setup(delay);

        return obj;
    }
}