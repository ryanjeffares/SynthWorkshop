using UnityEngine;

public class MathsMapCreator : IModuleCreator
{
    public GameObject CreateModule(GameObject prefab, Transform parent, Vector3 position, string[] input)
    {
        if (input.Length == 5)
        {
            if (!float.TryParse(input[1], out var inMin))
            {
                return null;
            }

            if (!float.TryParse(input[2], out var inMax))
            {
                return null;
            }

            if (!float.TryParse(input[3], out var outMin))
            {
                return null;
            }

            if (!float.TryParse(input[4], out var outMax))
            {
                return null;
            }

            var map = Object.Instantiate(prefab, position, Quaternion.identity, parent);
            map.GetComponent<MathsModuleController>().SetType(MathsSign.Map, AudioCV.CV);
            map.GetComponent<MathsModuleController>().SetupMap(inMin, inMax, outMin, outMax);
            return map;
        }

        return null;
    }
}