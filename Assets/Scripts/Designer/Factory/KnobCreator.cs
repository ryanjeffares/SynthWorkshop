using UnityEngine;

public class KnobCreator : IModuleCreator
{
    public GameObject CreateModule(GameObject prefab, Transform parent, Vector3 position, string[] input)
    {
        GameObject knob;

        switch (input.Length)
        {
            case 1:
                knob = Object.Instantiate(prefab, position, Quaternion.identity, parent);
                knob.GetComponent<KnobModuleController>().SetValues(0, 1, 0.5);
                return knob;
            case 4:
                if (!double.TryParse(input[1], out var min))
                {
                    return null;
                }

                if (!double.TryParse(input[2], out var max))
                {
                    return null;
                }

                if (!double.TryParse(input[3], out var value))
                {
                    return null;
                }

                knob = Object.Instantiate(prefab, position, Quaternion.identity, parent);
                knob.GetComponent<KnobModuleController>().SetValues(min, max, value);
                return knob;
        }

        return null;
    }
}