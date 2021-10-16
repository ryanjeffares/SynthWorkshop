using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public enum AudioCV
{
    Audio, CV, Either
}

public enum InputOutput
{
    Input, Output
}

public class IOModuleController : ModuleParent
{
    private InputOutput _inputOutput;
    public InputOutput InputOutput
    {
        get => _inputOutput;
        set
        {
            _inputOutput = value;
            if(InputOutput == InputOutput.Output)
            {
                connectors[2].transform.parent.gameObject.SetActive(false);
                connectors[3].transform.parent.gameObject.SetActive(false);
            }
            else
            {
                connectors[0].transform.parent.gameObject.SetActive(false);
                connectors[1].transform.parent.gameObject.SetActive(false);
            }
        }
    }

    private AudioCV _audioCv;
    public AudioCV AudioCv
    {
        get => _audioCv;
        set
        {
            _audioCv = value;
            GetComponent<Image>().color = AudioCv switch
            {
                AudioCV.Audio => "#F7FFAE".ColourFromHex(),
                AudioCV.CV => "#AEFFF8".ColourFromHex(),
                _ => Color.white,
            };
            nameText.text = AudioCv.ToString();            
        }
    }

    protected override void ChildAwake()
    {
        moduleType = ModuleType.IOModule;
    }

    public override List<ModuleException> CheckErrors()
    {
        var exceptions = new List<ModuleException>();
        if(connectors.All(c => !c.isConnected))
        {
            var exception = new ModuleException("IO Node is not connected to anything and will be ignored.", ModuleException.SeverityLevel.Warning);
            exceptions.Add(exception);
        }
        return exceptions;
    }

    public void SetType(AudioCV t, InputOutput i)
    {
        AudioCv = t;
        InputOutput = i;
        if (InputOutput == InputOutput.Input)
        {
            connectors[2].SetTypes(AudioCv, InputOutput.Output);
            connectors[3].SetTypes(AudioCv, InputOutput.Output);
        }
        else
        {
            connectors[0].SetTypes(AudioCv, InputOutput.Input);
            connectors[1].SetTypes(AudioCv, InputOutput.Input);
        } 
        if(AudioCv == AudioCV.CV)
        {
            if(InputOutput == InputOutput.Input)
            {
                for(int idx = 0; idx < connectors.Count; idx++)
                {
                    connectors[idx].transform.parent.gameObject.SetActive(idx == 3);
                }
                connectors[3].transform.parent.localPosition = new Vector3(0, -75);
                connectors[3].transform.parent.GetComponentInChildren<Text>().gameObject.SetActive(false);
            }
            else
            {
                for (int idx = 0; idx < connectors.Count; idx++)
                {
                    connectors[idx].transform.parent.gameObject.SetActive(idx == 0);
                }
                connectors[0].transform.parent.localPosition = new Vector3(0, 75);
                connectors[0].transform.parent.GetComponentInChildren<Text>().gameObject.SetActive(false);
            }
        }
    }  
    
    public void CreateJsonEntry(Dictionary<string, object> jsonDict, Dictionary<ModuleConnectorController, int> outputLookup)
    {
        if(AudioCv == AudioCV.Audio)
        {
            if (InputOutput == InputOutput.Output)
            {
                var leftInputNums = connectors[0].connectedModuleConnectors.Select(c => outputLookup[c]);
                var rightInputNums = connectors[1].connectedModuleConnectors.Select(c => outputLookup[c]);
                jsonDict.Add("left_input_from", leftInputNums);
                jsonDict.Add("right_input_from", rightInputNums);
            }
        }
        jsonDict.Add("position", transform.localPosition);
    }
}
