using UnityEngine;

public class FilterCreator : IModuleCreator
{
    public GameObject CreateModule(GameObject prefab, Transform parent, Vector3 position, string[] input)
    {
        switch (input.Length)
        {
            case 1:                
                return Object.Instantiate(prefab, position, Quaternion.identity, parent);
            case 4:
                if (!float.TryParse(input[1], out var freq))
                {
                    return null;
                }

                if (!float.TryParse(input[2], out var res))
                {
                    return null;
                }

                if (!int.TryParse(input[3], out var type))
                {
                    return null;
                }

                var mod = Object.Instantiate(prefab, position, Quaternion.identity, parent);
                mod.GetComponent<FilterModuleController>().SetLabels(freq, res, type);
                return mod;
        }

        return null;
    }
}