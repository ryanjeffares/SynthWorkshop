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
    [SerializeField] private GameObject numberBoxPrefab;
    [SerializeField] private GameObject filterPrefab;
    [SerializeField] private GameObject togglePrefab;
    [SerializeField] private GameObject adsrPrefab;
    [SerializeField] private GameObject bangPrefab;
    [SerializeField] private GameObject bangDelayPrefab;
    [SerializeField] private GameObject soundfilePrefab;

    private static readonly Dictionary<string[], IModuleCreator> CreatorLookup = new Dictionary<string[], IModuleCreator>
    {
        {new[]{"saw~", "pulse~", "tri~", "sine~"}, new OscillatorCreator() },
        {new[]{"knob"}, new KnobCreator() },
        {new[]{"out~", "out", "in~", "in"}, new IOCreator() },
        {new[]{"+", "-", "*", "/", "sin", "cos", "tan", "asin", "acos", "atan", "abs", "int", "pow", "%", "mtof", "ftom"}, new MathsCreator() },
        {new[]{"+~", "-~", "*~", "/~", "sin~", "cos~", "tan~", "asin~", "acos~", "atan~", "abs~", "pow~", }, new AudioMathsCreator() },
        {new[]{"map"}, new MathsMapCreator() },
        {new[]{"num"}, new NumberBoxCreator() },
        {new[]{"filt~"}, new FilterCreator() },
        {new[]{"toggle"}, new ToggleCreator() },
        {new[]{"adsr~"}, new AdsrCreator() },
        {new[]{"bang"}, new BangCreator() },
        {new[]{"bdelay"}, new BangDelayCreator() },
        {new[]{"soundfile~"}, new SoundfileCreator() },
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
            {typeof(NumberBoxCreator), numberBoxPrefab},
            {typeof(FilterCreator), filterPrefab},            
            {typeof(ToggleCreator), togglePrefab},
            {typeof(AdsrCreator), adsrPrefab},
            {typeof(BangCreator), bangPrefab},
            {typeof(BangDelayCreator), bangDelayPrefab},
            {typeof(SoundfileCreator), soundfilePrefab},
        };
    }   



    public GameObject CreateModule(Transform parent, Vector3 position, string[] input)
    {
        input[0] = input[0].ToLower();
        var creator = CreatorLookup.FirstOrDefault(kvp => kvp.Key.Contains(input[0])).Value;

        if (creator != null)
        {
            var prefab = PrefabLookup[creator.GetType()];
            return creator.CreateModule(prefab, parent, position, input);
        }

        return null;
    }    
}