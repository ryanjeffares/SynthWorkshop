using UnityEngine;

public class NumberBoxCreator : IModuleCreator
{
    public GameObject CreateModule(GameObject prefab, Transform parent, Vector3 position, string[] input)
    {
        switch (input.Length)
        {
            case 1:
                {
                    var mod = Object.Instantiate(prefab, position, Quaternion.identity, parent);
                    mod.GetComponent<NumberModule>().SetNumber(0);
                    return mod;
                }
            case 2:
                {
                    if (!double.TryParse(input[1], out var val))
                    {
                        return null;
                    }

                    var mod = Object.Instantiate(prefab, position, Quaternion.identity, parent);
                    mod.GetComponent<NumberModule>().SetNumber(val);
                    return mod;
                }
        }

        return null;
    }
}