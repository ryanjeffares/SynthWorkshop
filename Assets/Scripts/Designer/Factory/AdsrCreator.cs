using UnityEngine;

public class AdsrCreator : IModuleCreator
{
    public GameObject CreateModule(GameObject prefab, Transform parent, Vector3 position, string[] input)
    {
        bool displayValues;
        float attack, decay, sustain, release;
        switch (input.Length)
        {
            case 1:
                attack = 0.1f;
                decay = 0.2f;
                sustain = 0.5f;
                release = 1;
                displayValues = false;
                break;
            case 5:
                if (!float.TryParse(input[1], out attack))
                {
                    return null;
                }

                if (!float.TryParse(input[2], out decay))
                {
                    return null;
                }

                if (!float.TryParse(input[3], out sustain))
                {
                    return null;
                }

                if (!float.TryParse(input[4], out release))
                {
                    return null;
                }

                displayValues = true;
                break;
            default: return null;
        }

        var obj = Object.Instantiate(prefab, position, Quaternion.identity, parent);
        obj.GetComponent<ADSRModuleController>().Setup(attack, decay, sustain, release, displayValues);
        return obj;
    }
}
