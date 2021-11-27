using UnityEngine;

public class OscillatorCreator : IModuleCreator
{
    public GameObject CreateModule(GameObject prefab, Transform parent, Vector3 position, string[] input)
    {
        if (input.Length > 2) return null;

        OscillatorType type;

        switch (input[0])
        {
            case "saw~":
                {
                    type = OscillatorType.Saw;
                    break;
                }
            case "pulse~":
                {
                    type = OscillatorType.Pulse;
                    break;
                }
            case "tri~":
                {
                    type = OscillatorType.Tri;
                    break;
                }
            case "sine~":
                {                    
                    type = OscillatorType.Sine;
                    break;
                }
            default: return null;
        }

        float frequency = 0;
        if (input.Length == 2)
        {
            if (!float.TryParse(input[1], out frequency))
            {
                return null;
            }
        }

        var osc = Object.Instantiate(prefab, position, Quaternion.identity, parent);
        osc.GetComponent<OscillatorModuleController>().SetType(type);
        if (frequency > 0)
        {
            osc.GetComponent<OscillatorModuleController>().SetFrequency(frequency);
        }

        return osc;
    }
}