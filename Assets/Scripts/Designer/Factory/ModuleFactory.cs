using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;

public class ModuleFactory : MonoBehaviour
{
    [SerializeField] private GameObject oscillatorPrefab;
    [SerializeField] private GameObject knobPrefab;
    [SerializeField] private GameObject ioPrefab;
    [SerializeField] private GameObject mathsPrefab;
    [SerializeField] private GameObject audioMathsPrefab;
    [SerializeField] private GameObject mathsMapPrefab;

    private static readonly Dictionary<string[], IModuleCreator> CreatorLookup = new Dictionary<string[], IModuleCreator>
    {
        {new[]{"saw~", "pulse~", "tri~", "sine~"}, new OscillatorCreator() },
        {new[]{"knob"}, new KnobCreator() },
        {new[]{"out~", "out", "in~", "in"}, new IOCreator() },
        {new[]{"+", "-", "*", "/", "sin", "cos", "tan", "asin", "acos", "atan", "abs", "int", "pow", "%", "mtof", "ftom"}, new MathsCreator() },
        {new[]{"+~", "-~", "*~", "/~", "sin~", "cos~", "tan~", "asin~", "acos~", "atan~", "abs~", "pow~", }, new AudioMathsCreator() },
        {new[]{"map"}, new MathsMapCreator() },
    };

    private static Dictionary<Type, GameObject> PrefabLookup;

    private void Awake()
    {
        PrefabLookup = new Dictionary<Type, GameObject>
        {
            {typeof(OscillatorCreator), oscillatorPrefab },
            {typeof(KnobCreator), knobPrefab},
            {typeof(IOCreator), ioPrefab },
            {typeof(MathsCreator), mathsPrefab},
            {typeof(AudioMathsCreator), audioMathsPrefab},
            {typeof(MathsMapCreator), mathsMapPrefab},
        };
    }

    public GameObject CreateModule(Transform parent, Vector3 position, string[] input)
    {        
        var creator = CreatorLookup.FirstOrDefault(kvp => kvp.Key.Contains(input[0])).Value;

        if (creator != null)
        {
            var prefab = PrefabLookup[creator.GetType()];
            return creator.CreateModule(prefab, parent, position, input);
        }

        return null;
    }    
}