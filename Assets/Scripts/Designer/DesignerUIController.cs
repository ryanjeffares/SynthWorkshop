using System;
using System.Linq;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using UnityEngine.UI;
using Newtonsoft.Json.Linq;

public class DesignerUIController : MonoBehaviour
{
    [SerializeField] private List<Button> mainButtons, oscillatorButtons, ioButtons, controlsButtons, utilButtons, mathsButtons;
    [SerializeField] private Button numberBoxButton;
    [SerializeField] private Button playButton, exportJsonButton, stopButton;
    [SerializeField] private Button closeErrors;
    [SerializeField] private List<GameObject> categoryScrollViews;
    [SerializeField] private AnimationCurve convex;
    [SerializeField] private GameObject mainContent;
    [SerializeField] private GameObject panelSliderPrefab, panelButtonPrefab, paramSliderPanelPrefab, panelTogglePrefab;
    [SerializeField] private GameObject errorScrollView, errorContent, errorPrefab;

    [SerializeField] private GameObject oscillatorModulePrefab, ioModulePrefab, knobModulePrefab, mathsModulePrefab, 
        audioMathsModulePrefab, mapModulePrefab, adsrModulePrefab, numberBoxPrefab, buttonModulePrefab, toggleModulePrefab;

    private Dictionary<GameObject, GameObject> _buttonCategoryLookup;
    private Dictionary<GameObject, bool> _categoryVisibleStates;
    private List<GameObject> _instantiatedModules;
    private List<GameObject> _instantiatedParamControls;
    private List<GameObject> _errors;

    private GameObject _paramPanel;

    private readonly struct CvParam
    {
        public enum ParamType
        {
            Slider, Button, Toggle
        }
        
        public readonly string name;
        public readonly int target;
        public readonly double min, max;
        public readonly ParamType paramType; 

        public CvParam(string n, int t, double mn, double mx, ParamType pt)
        {
            name = n;
            target = t;
            min = mn;
            max = mx;
            paramType = pt;
        }
    }

    private void Awake()
    {
        ModuleParent.ModuleDestroyed += ModuleDestroyedCallback;
        _instantiatedParamControls = new List<GameObject>();
        _instantiatedModules = new List<GameObject>();
        _buttonCategoryLookup = new Dictionary<GameObject, GameObject>();
        _categoryVisibleStates = new Dictionary<GameObject, bool>();
        _errors = new List<GameObject>();
        for(int i = 0; i < mainButtons.Count; i++)
        {
            var b = mainButtons[i];
            _buttonCategoryLookup.Add(b.gameObject, categoryScrollViews[i]);
            b.onClick.AddListener(() => MainButtonCallback(b));
            _categoryVisibleStates.Add(categoryScrollViews[i], false);
        }
        foreach(var b in oscillatorButtons)
        {
            b.onClick.AddListener(() => OscillatorButtonCallback(b));
        }
        foreach(var b in ioButtons)
        {
            b.onClick.AddListener(() => IOButtonCallback(b));
        }

        foreach (var b in controlsButtons)
        {
            b.onClick.AddListener(() => ControlsButtonCallback(b));
        }
        foreach(var b in mathsButtons)
        {
            b.onClick.AddListener(() => MathsButtonCallback(b));
        }
        foreach (var b in utilButtons)
        {
            b.onClick.AddListener(() => UtilButtonCallback(b));            
        }
        numberBoxButton.onClick.AddListener(NumberBoxButtonCallback);
        playButton.onClick.AddListener(() => ExportArrangement(true));
        stopButton.onClick.AddListener(() => SynthWorkshopLibrary.StopAudio());
        exportJsonButton.onClick.AddListener(() => ExportArrangement(false));
        closeErrors.onClick.AddListener(() => errorScrollView.SetActive(false));
    }

    private void Start()
    {
        SynthWorkshopLibrary.HelloWorld();
        SynthWorkshopLibrary.HelloWorldFromMain();
    }

    private void OnDestroy()
    {
        ModuleParent.ModuleDestroyed -= ModuleDestroyedCallback;
    }

    /// <summary>
    /// Invoked when a Module is destroyed, so we can remove it from our list of instantiated modules.
    /// </summary>
    /// <param name="mod">The module that was destroyed.</param>
    private void ModuleDestroyedCallback(GameObject mod)
    {
        _instantiatedModules.Remove(mod);
    }

    /// <summary>
    /// Callback for the category buttons: IO, Oscillators, Controls, etc.
    /// </summary>
    /// <param name="button">The button that was clicked.</param>
    private void MainButtonCallback(Button button)
    {    
        var g = button.gameObject;
        foreach (var kvp in _buttonCategoryLookup.Where(k => k.Key != g))
        {
            if (!_categoryVisibleStates[kvp.Value]) continue;
            var cat = _categoryVisibleStates.First(c => c.Key == kvp.Value).Key;
            StartCoroutine(cat.InterpolatePosition(new Vector3(-100, 0), 0.5f, convex));
            _categoryVisibleStates[kvp.Value] = false;
        }
        StartCoroutine(_buttonCategoryLookup[g].InterpolatePosition(new Vector3(_categoryVisibleStates[_buttonCategoryLookup[g]] ? -100 : 100, 0), 0.5f, convex));
        _categoryVisibleStates[_buttonCategoryLookup[g]] = !_categoryVisibleStates[_buttonCategoryLookup[g]];
    }

    private void OscillatorButtonCallback(Button b)
    {
        var module = Instantiate(oscillatorModulePrefab, mainContent.transform);
        module.GetComponent<OscillatorModuleController>().SetType((OscillatorType)oscillatorButtons.IndexOf(b));
        _instantiatedModules.Add(module);
    }

    private void IOButtonCallback(Button b)
    {
        var module = Instantiate(ioModulePrefab, mainContent.transform);
        var idx = ioButtons.IndexOf(b);
        module.GetComponent<IOModuleController>().SetType(idx < 2 ? AudioCV.Audio : AudioCV.CV, idx % 2 == 0 ? InputOutput.Output : InputOutput.Input);
        _instantiatedModules.Add(module);
    }
    
    private void ControlsButtonCallback(Button b)
    {
        switch (controlsButtons.IndexOf(b))
        {
            case 0:
                _instantiatedModules.Add(Instantiate(knobModulePrefab, mainContent.transform));
                break;
            case 1:
                _instantiatedModules.Add(Instantiate(buttonModulePrefab, mainContent.transform));
                break;
            case 2:
                _instantiatedModules.Add(Instantiate(toggleModulePrefab, mainContent.transform));
                break;
        }
    }

    private void MathsButtonCallback(Button b)
    {
        var idx = mathsButtons.IndexOf(b);
        var audioCv  = idx > 16 ? AudioCV.Audio : AudioCV.CV;
        var sign = (MathsSign)(idx % 17);
        var module = Instantiate(audioCv == AudioCV.Audio ? audioMathsModulePrefab : idx == 14 ? mapModulePrefab : mathsModulePrefab, mainContent.transform);
        module.GetComponent<MathsModuleController>().SetType(sign, audioCv);
        _instantiatedModules.Add(module);
    }

    private void UtilButtonCallback(Button b)
    {
        _instantiatedModules.Add(Instantiate(adsrModulePrefab, mainContent.transform));
    }

    private void NumberBoxButtonCallback()
    {
        _instantiatedModules.Add(Instantiate(numberBoxPrefab, mainContent.transform));
    }

    /// <summary>
    /// Function for creating a JSON from the current arrangement of modules.
    /// </summary>
    private void ExportArrangement(bool play)
    {
        if (ErrorCheck()) return;

        int controlModuleCount = 1;
        int oscillatorModuleCount = 1;
        int ioModuleCount = 1;
        int mathsModuleCount = 1;
        int numberBoxCount = 1;
        int adsrCount = 1;

        int outputId = 0;

        var json = new Dictionary<string, Dictionary<string, Dictionary<string, object>>>
        {
            {"ControlModules", new Dictionary<string, Dictionary<string, object>>()},
            {"OscillatorModules", new Dictionary<string, Dictionary<string, object>>()},
            {"IOModules", new Dictionary<string, Dictionary<string, object>>()},
            {"MathsModules", new Dictionary<string, Dictionary<string, object>>()},
            {"NumberBoxes", new Dictionary<string, Dictionary<string, object>>()},
            {"ADSRModules", new Dictionary<string, Dictionary<string, object>>()}
        };
        var usedModules = _instantiatedModules.Where(m => m.GetComponent<ModuleParent>().CheckIfUsed())
            .Select(m => m.GetComponent<ModuleParent>()).ToList();
        var namedParams = new List<CvParam>();
        // find list of used outputs, and assign them a unique integer id
        // need to do non-number boxes first, as the number boxes are going to not increment the id so the value can "pass through" and not take up a new place in the map in c++
        var usedOutputs = new Dictionary<ModuleConnectorController, int>();
        foreach (var module in usedModules.Where(m => m.moduleType != ModuleType.NumberBox))
        {
            var connectors = module.GetUsedOutputs(out bool found);
            if (!found) continue;
            foreach (var c in connectors)
            {
                switch (module.moduleType)
                {
                    case ModuleType.KnobModule:
                    {
                        var knob = (KnobModuleController) module;
                        namedParams.Add(new CvParam(knob.Label, outputId, knob.min, knob.max, CvParam.ParamType.Slider));
                        break;
                    }
                    case ModuleType.ButtonModule:
                    {
                        var button = (ButtonModuleController) module;
                        namedParams.Add(new CvParam(button.Label, outputId, 0, 1, CvParam.ParamType.Button));
                        break;
                    }
                    case ModuleType.ToggleModule:
                    {
                        var toggle = (ToggleModuleController) module;
                        namedParams.Add(new CvParam(toggle.Label, outputId, 0, 1, CvParam.ParamType.Toggle));
                        break;
                    }
                }

                usedOutputs.Add(c, outputId);
                outputId++;
            }
        }

        // ... then we do the number boxes, and they just use whatever id their source had or a new id if they are a source
        foreach (var module in usedModules.Where(m => m.moduleType == ModuleType.NumberBox))
        {
            var connectors = module.GetUsedOutputs(out bool found);
            if (!found) continue;
            foreach (var c in connectors)
            {
                var mod = ((NumberModule) module).GetSourceConnector();
                if (mod != null)
                {
                    usedOutputs.Add(c, usedOutputs[mod]);
                }
                else
                {
                    usedOutputs.Add(c, outputId);
                    outputId++;
                }
            }
        }

        // go over the list again, now each input can get the id of where its getting its value/audio from
        // and create the json entries
        foreach (var module in usedModules)
        {
            var type = module.moduleType;
            switch (type)
            {
                case ModuleType.KnobModule:
                    CreateKnobJToken(ref controlModuleCount, json["ControlModules"], module as KnobModuleController, 
                        usedOutputs);
                    break;
                case ModuleType.ButtonModule:
                    CreateButtonJToken(ref controlModuleCount, json["ControlModules"], module as ButtonModuleController, 
                        usedOutputs);
                    break;
                case ModuleType.OscillatorModule:
                    CreateOscillatorJToken(ref oscillatorModuleCount, json["OscillatorModules"], module as OscillatorModuleController, usedOutputs);
                    break;
                case ModuleType.IOModule:
                    CreateIOJToken(ref ioModuleCount, json["IOModules"], module as IOModuleController, usedOutputs);
                    break;
                case ModuleType.MathsModule:
                    CreateMathsJToken(ref mathsModuleCount, json["MathsModules"], module as MathsModuleController, 
                        usedOutputs);
                    break;
                case ModuleType.NumberBox:
                    CreateNumberBox(ref numberBoxCount, json["NumberBoxes"], module as NumberModule, usedOutputs);
                    break;
                case ModuleType.ADSR:
                    CreateADSRJToken(ref adsrCount, json["ADSRModules"], module as ADSRModuleController, usedOutputs);
                    break;
            }
        }

        var jsonText = JToken.FromObject(json).ToString();
        File.WriteAllText(Application.dataPath + "/ArrangementJsons/test.json", jsonText);

        if (play)
        {
            PlayArrangement(jsonText, namedParams);
        }
    }

    private void PlayArrangement(string jsonText, List<CvParam> namedParams)
    {
        Debug.Log(SynthWorkshopLibrary.CreateModulesFromJson(jsonText));

        foreach (var s in _instantiatedParamControls)
        {
            Destroy(s);
        }

        _instantiatedParamControls.Clear();
        if (_paramPanel != null)
        {
            Destroy(_paramPanel);
        }

        _paramPanel = Instantiate(paramSliderPanelPrefab, transform);
        foreach (var p in namedParams)
        {
            switch (p.paramType)
            {
                case CvParam.ParamType.Slider:
                    var slider = Instantiate(panelSliderPrefab, _paramPanel.transform.GetChild(0).GetChild(0));
                    slider.GetComponent<ParamSliderController>().Setup(p.name, (float) p.min, (float) p.max, p.target);
                    _instantiatedParamControls.Add(slider);
                    break;
                case CvParam.ParamType.Button:
                    var button = Instantiate(panelButtonPrefab, _paramPanel.transform.GetChild(0).GetChild(0));
                    button.GetComponent<ParamButtonController>().Setup(p.name, p.target);
                    _instantiatedParamControls.Add(button);
                    break;
                case CvParam.ParamType.Toggle:
                    var toggle = Instantiate(panelTogglePrefab, _paramPanel.transform.GetChild(0).GetChild(0));
                    toggle.GetComponent<ParamToggleController>().Setup(p.name, p.target);
                    _instantiatedParamControls.Add(toggle);
                    break;
            }
        }
    }

    private void CreateNumberBox(ref int numberBoxCount, Dictionary<string, Dictionary<string, object>> json, 
        NumberModule module, Dictionary<ModuleConnectorController, int> usedOutputs)
    {
        var modName = $"NumberBox{numberBoxCount}";
        var mod = module.CreateJsonEntry(usedOutputs);
        json.Add(modName, mod);
        numberBoxCount++;
    }

    private void CreateIOJToken(ref int ioModuleCount, Dictionary<string, Dictionary<string, object>> json, 
        IOModuleController module, Dictionary<ModuleConnectorController, int> outputLookup)
    {
        var modName = $"IOModule{ioModuleCount}";
        var mod = new Dictionary<string, object>
        {
            {"type", $"{module.AudioCv}{module.InputOutput}"}
        };
        module.CreateJsonEntry(mod, outputLookup);
        ioModuleCount++;
        json.Add(modName, mod);        
    }

    private void CreateOscillatorJToken(ref int oscillatorModuleCount, Dictionary<string, Dictionary<string, object>> json, 
        OscillatorModuleController module, Dictionary<ModuleConnectorController, int> outputLookup)
    {
        var oscType = module.Type;
        var modName = $"OscillatorModule{oscillatorModuleCount}";
        var mod = new Dictionary<string, object>
        {
            {"type", oscType.ToString()}
        };
        module.CreateJsonEntry(mod, outputLookup);
        oscillatorModuleCount++;
        json.Add(modName, mod);
    }

    private void CreateKnobJToken(ref int controlModuleCount, Dictionary<string, Dictionary<string, object>> json, 
        KnobModuleController module, Dictionary<ModuleConnectorController, int> outputLookup)
    {
        var (min, max) = module.GetRange();
        var modName = $"ControlModule{controlModuleCount}";
        var mod = new Dictionary<string, object>
        {
            {"name", module.Label },
            {"type", "Knob"},
            {"min", min},
            {"max", max},
            {"outputId", module.GetOutputIdOfConnector(outputLookup)},
        };
        json.Add(modName, mod);
        controlModuleCount++;
    }
    
    private void CreateButtonJToken(ref int controlModuleCount, Dictionary<string, Dictionary<string, object>> jsonDict, 
        ButtonModuleController module, Dictionary<ModuleConnectorController, int> usedOutputs)
    {
        var modName = $"ControlModule{controlModuleCount}";
        var mod = new Dictionary<string, object>
        {
            {"name", module.Label},
            {"type", "Button"},
            {"outputId", module.GetOutputIdOfConnector(usedOutputs)}
        };
        jsonDict.Add(modName, mod);
        controlModuleCount++;
    }

    private void CreateMathsJToken(ref int mathsModuleCount, Dictionary<string, Dictionary<string, object>> json, 
        MathsModuleController module, Dictionary<ModuleConnectorController, int> outputLookup)
    {
        var modName = $"MathsModule{mathsModuleCount}";
        var mod = new Dictionary<string, object>
        {
            {"type", module.audioCv.ToString() }
        };
        module.CreateJsonEntry(mod, outputLookup);
        json.Add(modName, mod);
        mathsModuleCount++;
    }
    
    private void CreateADSRJToken(ref int adsrCount, Dictionary<string, Dictionary<string, object>> json,
        ADSRModuleController module, Dictionary<ModuleConnectorController, int> outputLookup)
    {
        var modName = $"ADSRModule{adsrCount}";
        var mod = new Dictionary<string, object>();
        module.CreateJsonEntry(mod, outputLookup);
        json.Add(modName, mod);
        adsrCount++;
    }

    // returns true if there are any errors that should halt exporting - will allow exporting with warnings only and display everything on the screen
    private bool ErrorCheck()
    {
        var exceptions = new List<ModuleParent.ModuleException>();
        foreach(var module in _instantiatedModules)
        {
            exceptions.AddRange(module.GetComponent<ModuleParent>().CheckErrors());
        }
        if (exceptions.Count == 0) 
        { 
            return false; 
        }
        errorScrollView.SetActive(true);
        if (_errors.Count > 0)
        {
            foreach (var e in _errors)
            {
                Destroy(e);
            }
            _errors.Clear();
        }
        foreach(var e in exceptions)
        {
            var errorMessage = Instantiate(errorPrefab, errorContent.transform);
            bool isError = e.severityLevel == ModuleParent.ModuleException.SeverityLevel.Error;
            errorMessage.GetComponent<Text>().text = (isError ? "ERROR: " : "WARNING: ") + e.message;
            errorMessage.GetComponent<Text>().color = isError ? Color.red : Color.yellow;
            _errors.Add(errorMessage);
        }
        return exceptions.Any(e => e.severityLevel == ModuleParent.ModuleException.SeverityLevel.Error);
    }
}
