using UnityEngine;

public class IOCreator : IModuleCreator
{
    public GameObject CreateModule(GameObject prefab, Transform parent, Vector3 position, string[] input)
    {
        if (input.Length != 1) return null;

        switch (input[0])
        {
            case "out~":
                {
                    var mod = Object.Instantiate(prefab, position, Quaternion.identity, parent);
                    mod.GetComponent<IOModuleController>().SetType(AudioCV.Audio, InputOutput.Output);
                    return mod;
                }
            default: return null;
        }
    }
}