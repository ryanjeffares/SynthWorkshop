using System.Linq;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using Newtonsoft.Json.Linq;
using System;

public class DesignerUIController : MonoBehaviour, IPointerDownHandler, IBeginDragHandler, IDragHandler
{
    private enum ModuleCategory
    {
        AudioMaths,
        Maths,
        Output,
        Oscillator,
        ADSR,
        NumberBox,
        Filter,
        Toggle,
        Bang,
        BangDelay,
        Soundfile
    }
    
    [SerializeField] private Button stopButton, clearButton, hideUiButton, centreButton;
    [SerializeField] private Button closeErrors;
    [SerializeField] private AnimationCurve convex;
    [SerializeField] private GameObject mainContent;
    [SerializeField] private GameObject errorScrollView, errorContent, errorPrefab;
    [SerializeField] private GameObject inputFieldPrefab;
    [SerializeField] private Slider masterVolume;
    [SerializeField] private Slider zoomSlider;
    [SerializeField] private Text fpsDisplay;
    [SerializeField] private Text instructions;

    private ModuleFactory _moduleFactory;

    private List<GameObject> _instantiatedModules;
    private List<GameObject> _errors;

    private readonly Dictionary<string, Dictionary<string, Dictionary<string, object>>> _currentArrangement =
        new Dictionary<string, Dictionary<string, Dictionary<string, object>>>();

    private int _totalModuleCount = 0;
    private int _controlModuleCount = 0;
    private int _oscillatorModuleCount = 0;
    private int _ioModuleCount = 0;
    private int _mathsModuleCount = 0;
    private int _numberBoxCount = 0;
    private int _adsrModuleCount = 0;
    private int _effectModuleCount = 0;
    private int _toggleModuleCount = 0;
    private int _bangModuleCount = 0;
    private int _soundfileModuleCount = 0;
    
    private int _soundOutputIndex = 0;
    private int _cvOutputIndex = 0;

    private bool _audioShouldStop;
    private bool _uiShowing = true;

    private void Awake()
    {
        Application.targetFrameRate = 60;
        ModuleParent.ModuleDestroyed += ModuleDestroyedCallback;

        _moduleFactory = GetComponent<ModuleFactory>();

        _instantiatedModules = new List<GameObject>();
        _errors = new List<GameObject>();

        stopButton.onClick.AddListener(() =>
        {
            _audioShouldStop = !_audioShouldStop;
            if (_audioShouldStop)
            {
                stopButton.GetComponentInChildren<Text>().text = "Resume";
                SynthWorkshopLibrary.StopAudio();
            }
            else
            {
                stopButton.GetComponentInChildren<Text>().text = "Stop";
                SynthWorkshopLibrary.ResumeAudio(); 
            }
        });
        closeErrors.onClick.AddListener(() => errorScrollView.SetActive(false));
        clearButton.onClick.AddListener(ClearAllModules);
        centreButton.onClick.AddListener(() => mainContent.transform.localPosition = Vector3.zero);
        hideUiButton.onClick.AddListener(() => 
        {
            _uiShowing = !_uiShowing;
            stopButton.gameObject.SetActive(_uiShowing);
            clearButton.gameObject.SetActive(_uiShowing);
            masterVolume.gameObject.SetActive(_uiShowing);
            instructions.gameObject.SetActive(_uiShowing);
            fpsDisplay.gameObject.SetActive(_uiShowing);
            zoomSlider.gameObject.SetActive(_uiShowing);
            centreButton.gameObject.SetActive(_uiShowing);
        });
        
        masterVolume.onValueChanged.AddListener(SynthWorkshopLibrary.SetMasterVolume);
        zoomSlider.onValueChanged.AddListener(val =>
        {
            mainContent.transform.localScale = new Vector3(val, val);
            zoomSlider.GetComponentInChildren<Text>().text = $"Zoom: {Math.Round(val, 2)}";
        });

        _currentArrangement["control_modules"] = new Dictionary<string, Dictionary<string, object>>();
        _currentArrangement["oscillator_modules"] = new Dictionary<string, Dictionary<string, object>>();
        _currentArrangement["io_modules"] = new Dictionary<string, Dictionary<string, object>>();
        _currentArrangement["maths_modules"] = new Dictionary<string, Dictionary<string, object>>();
        _currentArrangement["number_boxes"] = new Dictionary<string, Dictionary<string, object>>();
        _currentArrangement["adsr_modules"] = new Dictionary<string, Dictionary<string, object>>();
        _currentArrangement["effect_modules"] = new Dictionary<string, Dictionary<string, object>>();
        _currentArrangement["toggle_modules"] = new Dictionary<string, Dictionary<string, object>>();
        _currentArrangement["bang_modules"] = new Dictionary<string, Dictionary<string, object>>();
        _currentArrangement["bang_delay_modules"] = new Dictionary<string, Dictionary<string, object>>();
        _currentArrangement["soundfile_modules"] = new Dictionary<string, Dictionary<string, object>>();
    }

    private void Start()
    {
        SynthWorkshopLibrary.HelloWorld();
        SynthWorkshopLibrary.HelloWorldFromMain();
    }

    float _mouseScrollValue;

    private void Update()
    {
        var fps = 1f / Time.deltaTime;
        fpsDisplay.text = $"{Mathf.Round(fps)} FPS";

        var scroll = Input.mouseScrollDelta.y;
        if (scroll != _mouseScrollValue)
        {
            scroll *= 0.1f;            
            var scale = mainContent.transform.localScale;
            scale += new Vector3(scroll, scroll);            
            if (scale.x < 0.01f || scale.y < 0.01f)
            {
                scale = new Vector3(0.01f, 0.01f);
            }
            if (scale.x > 3 || scale.y > 3)
            {
                scale = new Vector3(3, 3);
            }
            mainContent.transform.localScale = scale;
            zoomSlider.SetValueWithoutNotify(scale.x);
            zoomSlider.GetComponentInChildren<Text>().text = $"Zoom: {Math.Round(scale.x, 2)}";
        }
        _mouseScrollValue = scroll;
    }

    private void OnDestroy()
    {
        ModuleParent.ModuleDestroyed -= ModuleDestroyedCallback;
    }

    private Vector2 _mousePos;

    public void OnBeginDrag(PointerEventData eventData)
    {
        _mousePos = eventData.position;
    }

    public void OnDrag(PointerEventData eventData)
    {
        var diff = eventData.position - _mousePos;
        mainContent.transform.position += (Vector3)diff;
        _mousePos = eventData.position;
    }

    private bool _firstClickReceived;
    private bool _secondClickReceived;

    public void OnPointerDown(PointerEventData eventData)
    {
        if (!_firstClickReceived)
        {
            _firstClickReceived = true;
            StartCoroutine(WaitForDoubleClick(eventData));
        }
        else
        {
            if (!_secondClickReceived)
            {
                _secondClickReceived = true;
            }
        }
    }

    private IEnumerator WaitForDoubleClick(PointerEventData eventData)
    {
        yield return new WaitForSeconds(0.2f);

        if (_secondClickReceived)
        {
            var inputField = Instantiate(inputFieldPrefab, eventData.position, Quaternion.identity, mainContent.transform).GetComponent<TextInputField>();
            inputField.Select();
            inputField.onSubmit.AddListener(input => OnInputSubmit(inputField.gameObject, inputField.GetText(), eventData.position));
        }

        _firstClickReceived = false;
        _secondClickReceived = false;
    }

    private void OnInputSubmit(GameObject inputField, string input, Vector3 position)
    {
        if (input.Equals("delete", System.StringComparison.OrdinalIgnoreCase))
        {
            Destroy(inputField);
            return;
        }

        var mod = _moduleFactory.CreateModule(mainContent.transform, position, input.Split(' '));       

        if (mod != null)
        {
            _instantiatedModules.Add(mod);

            if (mod.TryGetComponent<OscillatorModuleController>(out var o))
            {
                OscillatorCreated(o.Type);
            }
            else if (mod.TryGetComponent<KnobModuleController>(out _))
            {
                KnobCreated();
            }
            else if (mod.TryGetComponent<IOModuleController>(out var i))
            {
                IOCreated(i.AudioCv, i.InputOutput);
            }
            else if (mod.TryGetComponent<MathsModuleController>(out var m))
            {
                MathsCreated(m);
            }
            else if (mod.TryGetComponent<NumberModule>(out var n))
            {
                NumberBoxCreated(n);
            }
            else if (mod.TryGetComponent<FilterModuleController>(out var f))
            {
                FilterCreated(f);
            }
            else if (mod.TryGetComponent<ToggleModuleController>(out var t))
            {
                ToggleCreated(t);
            }
            else if (mod.TryGetComponent<ADSRModuleController>(out var a))
            {
                AdsrCreated(a);
            }
            else if (mod.TryGetComponent<BangModuleController>(out var b))
            {
                BangCreated(b);
            }
            else if (mod.TryGetComponent<BangDelayModuleController>(out var bd))
            {
                BangDelayCreated(bd);
            }
            else if (mod.TryGetComponent<SoundfileModuleController>(out var sf))
            {
                SoundfileCreated(sf);
            }

            Destroy(inputField);
            return;
        }        

        inputField.GetComponent<TextInputField>().SetTextColour(Color.red);
    }

    private void OscillatorCreated(OscillatorType type)
    {
        var osc = _instantiatedModules.Last().GetComponent<OscillatorModuleController>();
        osc.SetOutputIndexes(_soundOutputIndex, _cvOutputIndex);

        var dict = new Dictionary<string, object>
        {
            {"type_string", type.ToString() },
            {"type_int", (int) type },
            {"global_index", _totalModuleCount },
            {"sound_output_to", _soundOutputIndex++ },
            {"cv_out_to", _cvOutputIndex++ },
            {"initial_frequency", osc.Frequency }
        };

        var id = $"oscillator_module_{_oscillatorModuleCount}";
        _currentArrangement["oscillator_modules"].Add(id, dict);
        osc.SetIdentifier(id, _totalModuleCount);

        _oscillatorModuleCount++;
        _totalModuleCount++;

        SynthWorkshopLibrary.CreateNewModule((int)ModuleCategory.Oscillator, JObject.FromObject(dict).ToString());
    }

    private void KnobCreated()
    {
        var outIdx = _cvOutputIndex;
        var dict = new Dictionary<string, object>
        {
            {"global_index", _totalModuleCount },
            {"output_id", outIdx },
            {"type", "Knob"}
        };

        var knobName = $"control_module_{_controlModuleCount}";
        _currentArrangement["control_modules"].Add(knobName, dict);

        _instantiatedModules.Last().GetComponent<ModuleParent>().SetIdentifier(knobName, _totalModuleCount);

        var knobController = _instantiatedModules.Last().GetComponent<KnobModuleController>();
        knobController.onLabelChanged.AddListener(s => _currentArrangement["control_modules"][knobName]["name"] = s);
        knobController.onMinChanged.AddListener(m => _currentArrangement["control_modules"][knobName]["min"] = m);
        knobController.onMaxChanged.AddListener(m => _currentArrangement["control_modules"][knobName]["max"] = m);
        knobController.onSkewChanged.AddListener(s => _currentArrangement["control_modules"][knobName]["skew"] = s);
        knobController.onSliderMoved.AddListener(v =>
        {
            _currentArrangement["control_modules"][knobName]["initial"] = v;
            SynthWorkshopLibrary.SetCvParam(outIdx, v);
        });
        knobController.SetOutputIndex(outIdx);

        SynthWorkshopLibrary.CreateCvBufferWithKey(outIdx);

        _controlModuleCount++;
        _cvOutputIndex++;
        _totalModuleCount++;
    }

    private void IOCreated(AudioCV audioCV, InputOutput io)
    {
        var dict = new Dictionary<string, object>
        {
            {"type", $"{audioCV}{io}" },
            {"global_index", _totalModuleCount }
        };

        var id = $"io_module_{_ioModuleCount}";
        _currentArrangement["io_modules"].Add(id, dict);
        _instantiatedModules.Last().GetComponent<ModuleParent>().SetIdentifier(id, _totalModuleCount);

        _ioModuleCount++;
        _totalModuleCount++;

        SynthWorkshopLibrary.CreateNewModule((int)ModuleCategory.Output, JObject.FromObject(dict).ToString());
    }

    private void MathsCreated(MathsModuleController moduleController)
    {
        var outIdx = _cvOutputIndex;
        moduleController.SetOutputIndex(outIdx);

        var dict = new Dictionary<string, object>
        {
            {"global_index", _totalModuleCount },
            {"type_string", moduleController.audioCv.ToString() },
            {"type_int", (int) moduleController.mathsSign },
            {"operator", moduleController.mathsSign.ToString()},
            {"output_id", outIdx }
        };

        if (moduleController.mathsSign == MathsSign.Map)
        {
            dict["max_in"] = moduleController.maxIn;
            dict["max_out"] = moduleController.maxOut;
            dict["min_in"] = moduleController.minIn;
            dict["min_out"] = moduleController.minOut;
        }
        else
        {
            dict["initial_value"] = moduleController.InitialValue;

            if (moduleController.audioCv == AudioCV.Audio)
            {
                dict["incoming_signal_type"] = moduleController.IncomingSignalType.ToString();
            }
        }

        var id = $"maths_module_{_mathsModuleCount}";
        _currentArrangement["maths_modules"].Add(id, dict);
        moduleController.SetIdentifier(id, _totalModuleCount);

        _mathsModuleCount++;
        _totalModuleCount++;
        _cvOutputIndex++;

        var cat = (int)(moduleController.audioCv == AudioCV.Audio ? ModuleCategory.AudioMaths : ModuleCategory.Maths);
        SynthWorkshopLibrary.CreateNewModule(cat, JObject.FromObject(dict).ToString());
    }

    private void NumberBoxCreated(NumberModule mod)
    {
        var outIdx = _cvOutputIndex;
        mod.SetOutputIndex(outIdx);

        var dict = new Dictionary<string, object>
        {
            {"global_index", _totalModuleCount },
            {"output_to", outIdx },
            {"initial_value", mod.CurrentValue }
        };

        var id = $"number_box_{_numberBoxCount++}";
        _currentArrangement["number_boxes"].Add(id, dict);
        mod.SetIdentifier(id, _totalModuleCount);

        _totalModuleCount++;
        _cvOutputIndex++;

        SynthWorkshopLibrary.CreateNewModule((int)ModuleCategory.NumberBox, JObject.FromObject(dict).ToString());
    }

    private void FilterCreated(FilterModuleController mod)
    {
        mod.SetOutputIndex(_soundOutputIndex);

        var dict = new Dictionary<string, object>
        {
            {"global_id", _totalModuleCount },
            {"output_id", _soundOutputIndex },
            {"cutoff", mod.Cutoff },
            {"q", mod.Resonance },
            {"type", mod.FilterType },
        };

        var id = $"effect_{_effectModuleCount++}";
        _currentArrangement["effect_modules"].Add(id, dict);
        mod.SetIdentifier(id, _totalModuleCount);

        _totalModuleCount++;
        _soundOutputIndex++;

        SynthWorkshopLibrary.CreateNewModule((int)ModuleCategory.Filter, JObject.FromObject(dict).ToString());
    }

    private void ToggleCreated(ToggleModuleController toggle)
    {
        var globalId = _totalModuleCount;

        var dict = new Dictionary<string, object>
        {
            {"global_index", globalId},
        };

        var id = $"toggle_{_toggleModuleCount++}";
        _currentArrangement["toggle_modules"].Add(id, dict);

        toggle.SetIdentifier(id, globalId);
        toggle.toggleChanged.AddListener(state =>
        {
            SynthWorkshopLibrary.TriggerCallback(globalId, state);
        });

        _totalModuleCount++;

        SynthWorkshopLibrary.CreateNewModule((int)ModuleCategory.Toggle, JObject.FromObject(dict).ToString());
    }

    private void AdsrCreated(ADSRModuleController moduleController)
    {
        moduleController.SetOutputIndex(_soundOutputIndex);

        var dict = new Dictionary<string, object>
        {
            {"global_index", _totalModuleCount },
            {"output_to", _soundOutputIndex },
            {"attack", moduleController.Attack },
            {"decay", moduleController.Decay },
            {"sustain", moduleController.Sustain },
            {"release", moduleController.Release }
        };

        var id = $"adsr_{_adsrModuleCount++}";
        _currentArrangement["adsr_modules"].Add(id, dict);
        moduleController.SetIdentifier(id, _totalModuleCount);

        _totalModuleCount++;
        _soundOutputIndex++;

        SynthWorkshopLibrary.CreateNewModule((int)ModuleCategory.ADSR, JObject.FromObject(dict).ToString());
    }

    private void BangCreated(BangModuleController bangController)
    {
        var globalId = _totalModuleCount;

        var dict = new Dictionary<string, object>
        {
            {"global_index", globalId }
        };

        var id = $"bang_{_bangModuleCount++}";
        _currentArrangement["bang_modules"].Add(id, dict);
        bangController.SetIdentifier(id, globalId);
        bangController.buttonPressed.AddListener(() =>
        {
            SynthWorkshopLibrary.TriggerCallback(globalId, true);
        });

        _totalModuleCount++;

        SynthWorkshopLibrary.CreateNewModule((int)ModuleCategory.Bang, JObject.FromObject(dict).ToString());
    }

    private void BangDelayCreated(BangDelayModuleController bangDelayModule)
    {
        var globalId = _totalModuleCount;

        var dict = new Dictionary<string, object>
        {
            {"global_index", globalId },
            {"delay", bangDelayModule.Delay },
        };

        var id = $"bang_delay_{_bangModuleCount++}";
        _currentArrangement["bang_delay_modules"].Add(id, dict);
        bangDelayModule.SetIdentifier(id, globalId);
        _totalModuleCount++;
        SynthWorkshopLibrary.CreateNewModule((int)ModuleCategory.BangDelay, JObject.FromObject(dict).ToString());
    }

    private void SoundfileCreated(SoundfileModuleController soundfileModule)
    {
        var globalId = _totalModuleCount;
        var soundOut = _soundOutputIndex;

        var dict = new Dictionary<string, object>
        {
            {"global_index", globalId },
            {"output_index", soundOut }
        };

        var id = $"soundfile_module_{_soundfileModuleCount++}";
        _currentArrangement["soundfile_modules"].Add(id, dict);
        soundfileModule.SetIdentifier(id, globalId);
        soundfileModule.SetOutputIndex(soundOut);

        _totalModuleCount++;
        _soundOutputIndex++;

        SynthWorkshopLibrary.CreateNewModule((int)ModuleCategory.Soundfile, JObject.FromObject(dict).ToString());
    }

    /// <summary>
    /// Invoked when a Module is destroyed, so we can remove it from our list of instantiated modules.
    /// </summary>
    /// <param name="mod">The module that was destroyed.</param>
    private void ModuleDestroyedCallback(GameObject mod)
    {
        _instantiatedModules.Remove(mod);
    }    

    private void ClearAllModules()
    {
        foreach (var m in _instantiatedModules)
        {
            Destroy(m);
        }
        _instantiatedModules.Clear();
        SynthWorkshopLibrary.ClearAllModules();
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
