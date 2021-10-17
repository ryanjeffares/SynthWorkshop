using System;
using System.Linq;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using UnityEngine;
using UnityEngine.UI;
using UnityEditor;
using Newtonsoft.Json.Linq;

public class DesignerUIController : MonoBehaviour
{
    private enum ModuleCategory
    {
        AudioMaths,
        Maths,
        Output,
        Oscillator,
        ADSR
    }
    
    [SerializeField] private List<Button> mainButtons, oscillatorButtons, ioButtons, controlsButtons, utilButtons, mathsButtons;
    [SerializeField] private Button numberBoxButton;
    [SerializeField] private Button playButton, exportJsonButton, stopButton, clearButton, importButton;
    [SerializeField] private Button closeErrors;
    [SerializeField] private List<GameObject> categoryScrollViews;
    [SerializeField] private AnimationCurve convex;
    [SerializeField] private GameObject mainContent;
    [SerializeField] private GameObject panelSliderPrefab, panelButtonPrefab, paramSliderPanelPrefab, panelTogglePrefab;
    [SerializeField] private GameObject errorScrollView, errorContent, errorPrefab;
    [SerializeField] private Slider masterVolume;

    [SerializeField] private GameObject oscillatorModulePrefab, ioModulePrefab, knobModulePrefab, mathsModulePrefab, 
        audioMathsModulePrefab, mapModulePrefab, adsrModulePrefab, numberBoxPrefab, buttonModulePrefab, toggleModulePrefab;

    private Dictionary<GameObject, GameObject> _buttonCategoryLookup;
    private Dictionary<GameObject, bool> _categoryVisibleStates;
    private List<GameObject> _instantiatedModules;
    private List<GameObject> _instantiatedParamControls;
    private List<GameObject> _errors;

    private GameObject _paramPanel;

    private readonly Dictionary<string, Dictionary<string, Dictionary<string, object>>> _currentArrangement =
        new Dictionary<string, Dictionary<string, Dictionary<string, object>>>();

    private int _totalModuleCount = 0;
    private int _controlModuleCount = 0;
    private int _oscillatorModuleCount = 0;
    private int _ioModuleCount = 0;
    private int _mathsModuleCount = 0;
    private int _numberBoxCount = 0;
    private int _adsrModuleCount = 0;
    
    private int _soundOutputIndex = 0;
    private int _cvOutputIndex = 0;

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
        clearButton.onClick.AddListener(ClearAllModules);
        importButton.onClick.AddListener(() =>
        {
            var path = EditorUtility.OpenFilePanel("Choose .json file to import.", Path.Combine(Application.dataPath, "ArrangementJsons"), "json");
            ImportArrangement(path);
        });
        
        masterVolume.onValueChanged.AddListener(SynthWorkshopLibrary.SetMasterVolume);

        _currentArrangement["control_modules"] = new Dictionary<string, Dictionary<string, object>>();
        _currentArrangement["oscillator_modules"] = new Dictionary<string, Dictionary<string, object>>();
        _currentArrangement["io_modules"] = new Dictionary<string, Dictionary<string, object>>();
        _currentArrangement["maths_modules"] = new Dictionary<string, Dictionary<string, object>>();
        _currentArrangement["number_boxes"] = new Dictionary<string, Dictionary<string, object>>();
        _currentArrangement["adsr_modules"] = new Dictionary<string, Dictionary<string, object>>();
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
        var type = (OscillatorType) oscillatorButtons.IndexOf(b);
        var module = Instantiate(oscillatorModulePrefab, mainContent.transform);
        module.GetComponent<OscillatorModuleController>().SetType(type);
        _instantiatedModules.Add(module);

        var dict = new Dictionary<string, object>
        {
            {"type_string", type.ToString() },
            {"type_int", (int) type },
            {"global_index", _totalModuleCount++ },
            {"sound_output_to", _soundOutputIndex++ },
            {"cv_out_to", _cvOutputIndex++ },
        };

        var id = $"oscillator_module_{_oscillatorModuleCount}";
        _currentArrangement["oscillator_modules"].Add(id, dict);        
        _instantiatedModules.Last().GetComponent<ModuleParent>().SetIdentifier(id, _oscillatorModuleCount);
        _oscillatorModuleCount++;
        SynthWorkshopLibrary.CreateNewModule((int) ModuleCategory.Oscillator, JObject.FromObject(dict).ToString());
    }

    private void IOButtonCallback(Button b)
    {
        var module = Instantiate(ioModulePrefab, mainContent.transform);
        var idx = ioButtons.IndexOf(b);
        var controller =module.GetComponent<IOModuleController>(); 
        controller.SetType(idx < 2 ? AudioCV.Audio : AudioCV.CV, idx % 2 == 0 ? InputOutput.Output : InputOutput.Input);
        _instantiatedModules.Add(module);

        var dict = new Dictionary<string, object>
        {
            {"type", $"{controller.AudioCv}{controller.InputOutput}" },
            {"global_index", _totalModuleCount++ }
        };

        var id = $"io_module_{_ioModuleCount}";
        _currentArrangement["io_modules"].Add(id, dict);
        _instantiatedModules.Last().GetComponent<ModuleParent>().SetIdentifier(id, _ioModuleCount);
        _ioModuleCount++;
        SynthWorkshopLibrary.CreateNewModule((int) ModuleCategory.Output, JObject.FromObject(dict).ToString());
    }
    
    private void ControlsButtonCallback(Button b)
    {
        switch (controlsButtons.IndexOf(b))
        {
            case 0:
            {
                var knob = Instantiate(knobModulePrefab, mainContent.transform);
                _instantiatedModules.Add(knob);
                
                var dict = new Dictionary<string, object>
                {
                    {"global_index", _totalModuleCount++ },
                    {"output_id", _cvOutputIndex },
                    {"type", "Knob"}
                };

                var knobName = $"control_module_{_controlModuleCount}";
                _currentArrangement["control_modules"].Add(knobName, dict);
                
                _instantiatedModules.Last().GetComponent<ModuleParent>().SetIdentifier(knobName, _controlModuleCount);
                knob.GetComponent<KnobModuleController>().onLabelChanged.AddListener(s => _currentArrangement["control_modules"][knobName]["name"] = s);
                knob.GetComponent<KnobModuleController>().onMinChanged.AddListener(m => _currentArrangement["control_modules"][knobName]["min"] = m);
                knob.GetComponent<KnobModuleController>().onMaxChanged.AddListener(m => _currentArrangement["control_modules"][knobName]["max"] = m);
                
                SynthWorkshopLibrary.CreateCvBufferWithKey(_cvOutputIndex);
                
                _controlModuleCount++;
                _cvOutputIndex++;
                break;
            }
            case 1:
            {
                var button = Instantiate(buttonModulePrefab, mainContent.transform);
                _instantiatedModules.Add(button);
                
                var dict = new Dictionary<string, object>
                {
                    {"global_index", _totalModuleCount++ },
                    {"output_id", _cvOutputIndex++ },
                    {"type", "Button"}
                };

                var buttonName = $"control_module_{_controlModuleCount}";
                _currentArrangement["control_modules"].Add(buttonName, dict);
                _instantiatedModules.Last().GetComponent<ModuleParent>().SetIdentifier(buttonName, _controlModuleCount);
                _controlModuleCount++;
                button.GetComponent<ButtonModuleController>().onLabelChanged.AddListener(s => _currentArrangement["control_modules"][buttonName]["name"] = s);
                break;
            }
            case 2:
            {
                var toggle = Instantiate(toggleModulePrefab, mainContent.transform);
                _instantiatedModules.Add(toggle);
                
                var dict = new Dictionary<string, object>
                {
                    {"global_index", _totalModuleCount++ },
                    {"output_id", _cvOutputIndex++ },
                    {"type", "Toggle"}
                };

                var toggleName = $"control_module_{_controlModuleCount}";
                _currentArrangement["control_modules"].Add(toggleName, dict);
                _instantiatedModules.Last().GetComponent<ModuleParent>().SetIdentifier(toggleName, _controlModuleCount);
                _controlModuleCount++;
                toggle.GetComponent<ToggleModuleController>().onLabelChanged.AddListener(s => _currentArrangement["control_modules"][toggleName]["name"] = s);
                break;
            }
        }
    }

    private void MathsButtonCallback(Button b)
    {
        var idx = mathsButtons.IndexOf(b);
        var audioCv  = idx > 16 ? AudioCV.Audio : AudioCV.CV;
        var sign = (MathsSign)(idx % 17);
        var module = Instantiate(audioCv == AudioCV.Audio ? audioMathsModulePrefab : idx == 14 ? mapModulePrefab : mathsModulePrefab, mainContent.transform);
        var controller =module.GetComponent<MathsModuleController>(); 
        controller.SetType(sign, audioCv);
        
        _instantiatedModules.Add(module);

        var dict = new Dictionary<string, object>
        {
            {"global_index", _totalModuleCount++ },
            {"type_string", controller.audioCv.ToString() },
            {"type_int", (int) sign },
            {"operator", sign.ToString()},
            {"output_id", _cvOutputIndex++ }
        };

        var id = $"maths_module_{_mathsModuleCount}";
        _currentArrangement["maths_modules"].Add(id, dict);
        _instantiatedModules.Last().GetComponent<ModuleParent>().SetIdentifier(id, _mathsModuleCount);
        _mathsModuleCount++;
        SynthWorkshopLibrary.CreateNewModule((int) ModuleCategory.Maths, JObject.FromObject(dict).ToString());
    }

    private void UtilButtonCallback(Button b)
    {
        _instantiatedModules.Add(Instantiate(adsrModulePrefab, mainContent.transform));

        var dict = new Dictionary<string, object>
        {
            {"output_to", _soundOutputIndex++ },
            {"global_index", _totalModuleCount++ }
        };

        var id = $"adsr_module_{_adsrModuleCount}";
        _currentArrangement["adsr_modules"].Add(id, dict);
        _instantiatedModules.Last().GetComponent<ModuleParent>().SetIdentifier(id, _adsrModuleCount);
        _adsrModuleCount++;
        SynthWorkshopLibrary.CreateNewModule((int) ModuleCategory.ADSR, JObject.FromObject(dict).ToString());
    }

    private void NumberBoxButtonCallback()
    {
        _instantiatedModules.Add(Instantiate(numberBoxPrefab, mainContent.transform));
    }

    private void ClearAllModules()
    {
        foreach (var m in _instantiatedModules)
        {
            Destroy(m);
        }
        _instantiatedModules.Clear();
    }

    private readonly struct ParsedModule
    {
        public readonly GameObject obj;
        public readonly int output;
        
    }
    
    private void ImportArrangement(string path)
    {
        if (string.IsNullOrEmpty(path) || !File.Exists(path)) return;
        ClearAllModules();
        
        var json = JObject.Parse(File.ReadAllText(path));
        // get every module in the arrangement with its inputs/outputs
        // make the modules
        // connect the wires

        var lookup = new Dictionary<GameObject, JObject>();
        foreach (var category in json)
        {
            foreach (var module in category.Value)
            {
                try
                {
                    var contents = module.First as JObject;
                    switch (category.Key)
                    {
                        case "control_modules":
                        {
                            var type = contents["type"].ToString();
                            switch (type)
                            {
                                case "Knob":
                                    ControlsButtonCallback(controlsButtons[0]);
                                    _instantiatedModules.Last().GetComponent<KnobModuleController>()
                                        .SetValues(contents["name"].ToString(), contents["min"].ToObject<double>(),
                                            contents["max"].ToObject<double>());
                                    break;
                                case "Button":
                                    ControlsButtonCallback(controlsButtons[1]);
                                    _instantiatedModules.Last().GetComponent<ButtonModuleController>()
                                        .SetLabel(contents["name"].ToString());
                                    break;
                                case "Toggle":
                                    ControlsButtonCallback(controlsButtons[2]);
                                    _instantiatedModules.Last().GetComponent<ToggleModuleController>()
                                        .SetLabel(contents["name"].ToString());
                                    break;
                            }
                            break;
                        }
                        case "oscillator_modules":
                        {
                            OscillatorButtonCallback(oscillatorButtons[contents["type_int"].ToObject<int>()]);
                            break;
                        }
                        case "io_modules":
                        {
                            IOButtonCallback(ioButtons[contents["type"].ToString() switch
                            {
                                "AudioOutput" => 0,
                                "AudioInput" => 1,
                                "CVOutput" => 2,
                                "CVInput" => 3,
                                _ => throw new ArgumentException(contents["type"].ToString())
                            }]);
                            break;
                        }
                        case "maths_modules":
                        {
                            MathsButtonCallback(mathsButtons[contents["operator"].ToString() switch
                            {
                                "Plus" => 0,
                                "Minus" => 1,
                                "Multiply" => 2,
                                "Divide" => 3,
                                "Sin" => 4,
                                "Cos" => 5, 
                                "Tan" => 6, 
                                "Asin" => 7, 
                                "Acos" => 8, 
                                "Atan" => 9, 
                                "Abs" => 10, 
                                "Exp" => 11, 
                                "Int" => 12, 
                                "Mod" => 13, 
                                "Map" => 14, 
                                "Mtof" => 15, 
                                "Ftom" => 16,
                                _ => throw new ArgumentException(contents["operator"].ToString())
                            } + (contents["type_string"].ToString() == "Audio" ? 17 : 0)]);
                            break;
                        }
                        case "number_boxes":
                        {
                            NumberBoxButtonCallback();
                            _instantiatedModules.Last().GetComponent<NumberModule>()
                                .SetNumber(contents["initial_value"].ToObject<double>());
                            break;
                        }
                        case "adsr_modules":
                        {
                            UtilButtonCallback(utilButtons[0]);   
                            break;
                        }
                    }
                    _instantiatedModules.Last().transform.localPosition =
                        new Vector3(
                            contents["position"]["x"].ToObject<float>(),
                            contents["position"]["y"].ToObject<float>(),
                            contents["position"]["z"].ToObject<float>()
                        );
                    lookup.Add(_instantiatedModules.Last(), contents);
                }
                catch (Exception e)
                {
                    Debug.LogException(e);
                }
            }
        }

        foreach (var kvp in lookup)
        {
            var module = kvp.Key;
            var token = kvp.Value;
            
            switch (module.GetComponent<ModuleParent>().moduleType)
            {
                case ModuleType.OscillatorModule:
                    //module.GetComponent<OscillatorModuleController>().
                    break;
                case ModuleType.KnobModule:
                    break;
                case ModuleType.IOModule:
                    break;
                case ModuleType.MathsModule:
                    break;
                case ModuleType.NumberBox:
                    break;
                case ModuleType.ADSR:
                    break;
                case ModuleType.ButtonModule:
                    break;
                case ModuleType.ToggleModule:
                    break;
                default:
                    throw new ArgumentOutOfRangeException();
            }
        }
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

        // we already have a lot of this information, so just add in the things we dont
        foreach (var module in usedModules)
        {
            var type = module.moduleType;
            switch (type)
            {
                case ModuleType.ButtonModule:
                case ModuleType.KnobModule:
                case ModuleType.ToggleModule:
                    _currentArrangement["control_modules"][module.IdentifierName]["position"] =
                        JToken.FromObject(module.transform.localPosition);
                    break;
                /*case ModuleType.OscillatorModule:
                    CreateOscillatorJToken(ref oscillatorModuleCount, _currentArrangement["oscillator_modules"],
                        module as OscillatorModuleController, usedOutputs);
                    break;
                case ModuleType.IOModule:
                    CreateIOJToken(ref ioModuleCount, _currentArrangement["io_modules"], module as IOModuleController, usedOutputs);
                    break;
                case ModuleType.MathsModule:
                    CreateMathsJToken(ref mathsModuleCount, _currentArrangement["maths_modules"], module as MathsModuleController, 
                        usedOutputs);
                    break;
                case ModuleType.NumberBox:
                    CreateNumberBox(ref numberBoxCount, _currentArrangement["number_boxes"], module as NumberModule, usedOutputs);
                    break;
                case ModuleType.ADSR:
                    CreateADSRJToken(ref adsrCount, _currentArrangement["adsr_modules"], module as ADSRModuleController, usedOutputs);
                    break;*/
            }
        }

        var jsonText = JToken.FromObject(_currentArrangement).ToString();
        File.WriteAllText(Application.dataPath + "/ArrangementJsons/test.json", jsonText);

        if (play)
        {
            PlayArrangement(jsonText, namedParams);
        }
    }

    private void PlayArrangement(string jsonText, List<CvParam> namedParams)
    {
        var result = SynthWorkshopLibrary.CreateModulesFromJson(jsonText);
        Debug.Log(result);
        if (!result) return;

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
        var modName = $"number_box_{numberBoxCount}";
        var mod = module.CreateJsonEntry(usedOutputs);
        json.Add(modName, mod);
        numberBoxCount++;
    }

    private void CreateIOJToken(ref int ioModuleCount, Dictionary<string, Dictionary<string, object>> json, 
        IOModuleController module, Dictionary<ModuleConnectorController, int> outputLookup)
    {
        var modName = $"io_module_{ioModuleCount}";
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
        var modName = $"oscillator_module_{oscillatorModuleCount}";
        var mod = new Dictionary<string, object>
        {
            {"type_string", oscType.ToString() },
            {"type_int", (int)oscType }
        };
        module.CreateJsonEntry(mod, outputLookup);
        oscillatorModuleCount++;
        json.Add(modName, mod);
    }

    private void CreateKnobJToken(ref int controlModuleCount, Dictionary<string, Dictionary<string, object>> json, 
        KnobModuleController module, Dictionary<ModuleConnectorController, int> outputLookup)
    {
        var (min, max) = module.GetRange();
        var modName = $"control_module_{controlModuleCount}";
        var mod = new Dictionary<string, object>
        {
            {"name", module.Label },
            {"type", "Knob"},
            {"min", min},
            {"max", max},
            {"output_id", module.GetOutputIdOfConnector(outputLookup)},
            {"position", module.transform.localPosition}
        };
        json.Add(modName, mod);
        controlModuleCount++;
    }
    
    private void CreateButtonJToken(ref int controlModuleCount, Dictionary<string, Dictionary<string, object>> jsonDict, 
        ButtonModuleController module, Dictionary<ModuleConnectorController, int> usedOutputs)
    {
        var modName = $"control_module_{controlModuleCount}";
        var mod = new Dictionary<string, object>
        {
            {"name", module.Label},
            {"type", "Button"},
            {"output_id", module.GetOutputIdOfConnector(usedOutputs)},
            {"position", module.transform.localPosition}
        };
        jsonDict.Add(modName, mod);
        controlModuleCount++;
    }

    private void CreateToggleJToken(ref int controlModuleCount, Dictionary<string, Dictionary<string, object>> jsonDict,
        ToggleModuleController module, Dictionary<ModuleConnectorController, int> usedOutputs)
    {
        var modName = $"control_module_{controlModuleCount}";
        var mod = new Dictionary<string, object>
        {
            {"name", module.Label},
            {"type", "Toggle"},
            {"output_id", module.GetOutputIdOfConnector(usedOutputs)},
            {"position", module.transform.localPosition}
        };
        jsonDict.Add(modName, mod);
        controlModuleCount++;
    }
    private void CreateMathsJToken(ref int mathsModuleCount, Dictionary<string, Dictionary<string, object>> json, 
        MathsModuleController module, Dictionary<ModuleConnectorController, int> outputLookup)
    {
        var modName = $"maths_module_{mathsModuleCount}";
        var mod = new Dictionary<string, object>
        {
            {"type_string", module.audioCv.ToString() },
            {"type_int", (int)module.mathsSign }
        };
        module.CreateJsonEntry(mod, outputLookup);
        json.Add(modName, mod);
        mathsModuleCount++;
    }
    
    private void CreateADSRJToken(ref int adsrCount, Dictionary<string, Dictionary<string, object>> json,
        ADSRModuleController module, Dictionary<ModuleConnectorController, int> outputLookup)
    {
        var modName = $"adsr_module_{adsrCount}";
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
        if (_errors.Any())
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
