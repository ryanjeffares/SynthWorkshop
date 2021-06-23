using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Animations;

public enum MathsSign
{
    Plus, Minus, Multiply, Divide, Sin, Cos, Tan, Asin, Acos, Atan, Abs, Exp, Int, Mod, Map
}

public class MathsModuleController : ModuleParent
{
    public MathsSign mathsSign;
    public AudioCV audioCV;

    private AudioCV _incomingSignalType;

    protected override void ChildAwake()
    {
        moduleType = ModuleType.MathsModule;
    }

    public void SetType(MathsSign sign, AudioCV acv)
    {
        mathsSign = sign;
        audioCV = acv;
        switch (mathsSign)
        {
            case MathsSign.Plus:
                nameText.text = audioCV == AudioCV.Audio ? "+~" : "+";
                break;
            case MathsSign.Minus:
                nameText.text = audioCV == AudioCV.Audio ? "-~" : "-";
                break;
            case MathsSign.Multiply:
                nameText.text = audioCV == AudioCV.Audio ? "*~" : "*";
                break;
            case MathsSign.Divide:
                nameText.text = audioCV == AudioCV.Audio ? "/~" : "/";
                break;
            case MathsSign.Mod:
                nameText.text = "%";
                break;
            case MathsSign.Sin:
                nameText.text = audioCV == AudioCV.Audio ? "sin~" : "sin";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                connectors[0].gameObject.transform.parent.gameObject.transform.localPosition = new Vector3(0, 75);
                break;
            case MathsSign.Cos:
                nameText.text = audioCV == AudioCV.Audio ? "cos~" : "cos";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                connectors[0].gameObject.transform.parent.gameObject.transform.localPosition = new Vector3(0, 75);
                break;
            case MathsSign.Tan:
                nameText.text = audioCV == AudioCV.Audio ? "tan~" : "tan";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                connectors[0].gameObject.transform.parent.gameObject.transform.localPosition = new Vector3(0, 75);
                break;
            case MathsSign.Asin:
                nameText.text = audioCV == AudioCV.Audio ? "asin~" : "asin";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                connectors[0].gameObject.transform.parent.gameObject.transform.localPosition = new Vector3(0, 75);
                break;
            case MathsSign.Acos:
                nameText.text = audioCV == AudioCV.Audio ? "acos~" : "acos";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                connectors[0].gameObject.transform.parent.gameObject.transform.localPosition = new Vector3(0, 75);
                break;
            case MathsSign.Atan:
                nameText.text = audioCV == AudioCV.Audio ? "atan~" : "atan";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                connectors[0].gameObject.transform.parent.gameObject.transform.localPosition = new Vector3(0, 75);
                break;
            case MathsSign.Abs:
                nameText.text = audioCV == AudioCV.Audio ? "abs~" : "abs";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                connectors[0].gameObject.transform.parent.gameObject.transform.localPosition = new Vector3(0, 75);
                break;
            case MathsSign.Exp:
                nameText.text = audioCV == AudioCV.Audio ? "exp~" : "exp";
                break;
            case MathsSign.Int:
                nameText.text = "int";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                connectors[0].gameObject.transform.parent.gameObject.transform.localPosition = new Vector3(0, 75);
                break;
            case MathsSign.Map:
                nameText.text = "map";
                break;
            default:
                nameText.text = "Error Type";
                break;
        }
    }

    public void SetAudioOrCvIncoming(AudioCV acv, ModuleConnectorController moduleConnector)
    {
        if (moduleConnector == connectors[0]) return;
        _incomingSignalType = acv;
    }

    public void CreateJsonEntry(Dictionary<string, object> jsonDict, Dictionary<ModuleConnectorController, int> outputLookup)
    {
        jsonDict.Add("operator", mathsSign.ToString());
        if (connectors[0].isConnected)
        {
            jsonDict.Add("LeftInputFrom", outputLookup[connectors[0].sourceConnector]);
        }
        jsonDict.Add("IncomingSignalType", _incomingSignalType.ToString());
        if (mathsSign == MathsSign.Map)
        {
            jsonDict.Add("minIn", outputLookup[connectors[1].sourceConnector]);
            jsonDict.Add("maxIn", outputLookup[connectors[2].sourceConnector]);
            jsonDict.Add("minOut", outputLookup[connectors[3].sourceConnector]);
            jsonDict.Add("maxOut", outputLookup[connectors[4].sourceConnector]);
            if (outputLookup.ContainsKey(connectors[5]))
            {
                jsonDict.Add("outputId", outputLookup[connectors[5]]);
            }
        }
        else
        {
            if (connectors[1].isConnected) 
            { 
                jsonDict.Add("RightInputFrom", outputLookup[connectors[1].sourceConnector]); 
            }
            if (outputLookup.ContainsKey(connectors[2]))
            {
                jsonDict.Add("outputId", outputLookup[connectors[2]]);
            }   
        }
    }

    public override List<ModuleException> CheckErrors()
    {
        var exceptions = new List<ModuleException>();
        if (!connectors[0].isConnected)
        {
            var exception = new ModuleException("Maths node has no input and will be ignored.", ModuleException.SeverityLevel.Warning);
            exceptions.Add(exception);
        }
        if (connectors.All(c => !c.isConnected))
        {
            var exception = new ModuleException("Maths node not connected to anything and will be ignored.", ModuleException.SeverityLevel.Warning);
            exceptions.Add(exception);
        }
        if (!connectors.Last().isConnected)
        {
            var exception = new ModuleException("Maths node has no output and will be ignored.", ModuleException.SeverityLevel.Warning);
            exceptions.Add(exception);
        }
        if (mathsSign == MathsSign.Map)
        {
            if (connectors.GetRange(1, 4).Any(c => !c.isConnected))
            {
                var exception = new ModuleException("Map node does not have input for all values. You must set an input for every value.", ModuleException.SeverityLevel.Error);
                exceptions.Add(exception);
            }
        }
        else
        {
            if (connectors[1].gameObject.activeSelf && !connectors[1].isConnected)
            {
                var exception = new ModuleException("Maths node left input not connected to anything and will have no effect.", ModuleException.SeverityLevel.Warning);
                exceptions.Add(exception);
            }
            if (!connectors[2].isConnected)
            {
                var exception = new ModuleException("Maths node's output is not connected to anything, and will have no function.", ModuleException.SeverityLevel.Warning);
                exceptions.Add(exception);
            }   
        }
        return exceptions;
    }
}
