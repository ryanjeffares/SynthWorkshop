using System.Linq;
using System.Collections.Generic;
using UnityEngine;

public enum MathsSign
{
    Plus, Minus, Multiply, Divide, Sin, Cos, Tan, Asin, Acos, Atan, Abs, Pow, Int, Mod, Map, Mtof, Ftom
}

public class MathsModuleController : ModuleParent
{
    public MathsSign mathsSign;
    public AudioCV audioCv;

    public AudioCV IncomingSignalType { get; private set; } = AudioCV.Either;

    public float InitialValue { get; private set; }

    public float minIn, maxIn, minOut, maxOut;

    protected override void ChildAwake()
    {
        moduleType = ModuleType.MathsModule;
    }

    public void SetType(MathsSign sign, AudioCV acv, float initialValue)
    {
        mathsSign = sign;
        audioCv = acv;
        InitialValue = initialValue;

        switch (mathsSign)
        {
            case MathsSign.Plus:
                nameText.text = audioCv == AudioCV.Audio ? $"+~ {initialValue}" : $"+ {initialValue}";
                break;
            case MathsSign.Minus:
                nameText.text = audioCv == AudioCV.Audio ? $"-~ {initialValue}" : $"- {initialValue}";
                break;
            case MathsSign.Multiply:
                nameText.text = audioCv == AudioCV.Audio ? $"*~ {initialValue}" : $"* {initialValue}";
                break;
            case MathsSign.Divide:
                nameText.text = audioCv == AudioCV.Audio ? $"/~ {initialValue}" : $"/ {initialValue}";
                break;
            case MathsSign.Mod:
                nameText.text = $"% {initialValue}";
                break;
            case MathsSign.Sin:
                nameText.text = audioCv == AudioCV.Audio ? "sin~" : "sin";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Cos:
                nameText.text = audioCv == AudioCV.Audio ? "cos~" : "cos";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Tan:
                nameText.text = audioCv == AudioCV.Audio ? "tan~" : "tan";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Asin:
                nameText.text = audioCv == AudioCV.Audio ? "asin~" : "asin";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Acos:
                nameText.text = audioCv == AudioCV.Audio ? "acos~" : "acos";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Atan:
                nameText.text = audioCv == AudioCV.Audio ? "atan~" : "atan";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Abs:
                nameText.text = audioCv == AudioCV.Audio ? "abs~" : "abs";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Pow:
                nameText.text = audioCv == AudioCV.Audio ? $"pow~ {initialValue}" : $"pow {initialValue}";
                break;
            case MathsSign.Int:
                nameText.text = "int";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Map:
                nameText.text = "map";
                break;
            case MathsSign.Mtof:
                nameText.text = "mtof";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Ftom:
                nameText.text = "ftom";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            default:
                nameText.text = "Error Type";
                break;
        }
    }

    public void SetType(MathsSign sign, AudioCV acv)
    {
        mathsSign = sign;
        audioCv = acv;
        switch (mathsSign)
        {
            case MathsSign.Plus:
                nameText.text = audioCv == AudioCV.Audio ? "+~" : "+";
                break;
            case MathsSign.Minus:
                nameText.text = audioCv == AudioCV.Audio ? "-~" : "-";
                break;
            case MathsSign.Multiply:
                nameText.text = audioCv == AudioCV.Audio ? "*~" : "*";
                break;
            case MathsSign.Divide:
                nameText.text = audioCv == AudioCV.Audio ? "/~" : "/";
                break;
            case MathsSign.Mod:
                nameText.text = "%";
                break;
            case MathsSign.Sin:
                nameText.text = audioCv == AudioCV.Audio ? "sin~" : "sin";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Cos:
                nameText.text = audioCv == AudioCV.Audio ? "cos~" : "cos";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Tan:
                nameText.text = audioCv == AudioCV.Audio ? "tan~" : "tan";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Asin:
                nameText.text = audioCv == AudioCV.Audio ? "asin~" : "asin";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Acos:
                nameText.text = audioCv == AudioCV.Audio ? "acos~" : "acos";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Atan:
                nameText.text = audioCv == AudioCV.Audio ? "atan~" : "atan";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Abs:
                nameText.text = audioCv == AudioCV.Audio ? "abs~" : "abs";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Pow:
                nameText.text = audioCv == AudioCV.Audio ? "pow~" : "pow";
                break;
            case MathsSign.Int:
                nameText.text = "int";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Map:
                nameText.text = "map";
                break;
            case MathsSign.Mtof:
                nameText.text = "mtof";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            case MathsSign.Ftom:
                nameText.text = "ftom";
                connectors[1].gameObject.transform.parent.gameObject.SetActive(false);
                break;
            default:
                nameText.text = "Error Type";
                break;
        }
    }

    public void SetOutputIndex(int idx)
    {
        connectors[mathsSign == MathsSign.Map ? 5 : 2].SetOutputIndex(idx);
    }

    public void SetupMap(float inMin, float inMax, float outMin, float outMax)
    {
        minIn = inMin;
        maxIn = inMax;        
        minOut = outMin;
        maxOut = outMax;

        nameText.text = $"map {inMin} {inMax} {outMin} {outMax}";
    }

    public void SetAudioOrCvIncoming(AudioCV acv, ModuleConnectorController moduleConnector)
    {
        if (moduleConnector == connectors[0]) return;
        IncomingSignalType = acv;
        SynthWorkshopLibrary.SetAudioMathsIncomingSignal(GlobalIndex, (int)IncomingSignalType);
    }

    public void CreateJsonEntry(Dictionary<string, object> jsonDict, Dictionary<ModuleConnectorController, int> outputLookup)
    {
        jsonDict.Add("operator", mathsSign.ToString());
        
        if (connectors[0].isConnected)
        {
            jsonDict.Add("left_input_from", connectors[0].connectedModuleConnectors.Select(c => outputLookup[c]));
        }  
        
        jsonDict.Add("incoming_signal_type", IncomingSignalType.ToString());
        if (mathsSign == MathsSign.Map)
        {
            jsonDict.Add("min_in", outputLookup[connectors[1].sourceConnector]);
            jsonDict.Add("max_in", outputLookup[connectors[2].sourceConnector]);
            jsonDict.Add("min_out", outputLookup[connectors[3].sourceConnector]);
            jsonDict.Add("max_out", outputLookup[connectors[4].sourceConnector]);
            if (outputLookup.ContainsKey(connectors[5]))
            {
                jsonDict.Add("output_id", outputLookup[connectors[5]]);
            }
        }
        else
        {
            if (connectors[1].isConnected) 
            { 
                jsonDict.Add("right_input_from", connectors[1].connectedModuleConnectors.Select(c => outputLookup[c])); 
            }
            if (outputLookup.ContainsKey(connectors[2]))
            {
                jsonDict.Add("output_id", outputLookup[connectors[2]]);
            }   
        }
        jsonDict.Add("position", transform.localPosition);
    }

    public override List<ModuleException> CheckErrors()
    {
        var exceptions = new List<ModuleException>();
        if (!connectors[0].isConnected)
        {
            var exception = new ModuleException("Maths node has no input. You must set an input (left input connector).", ModuleException.SeverityLevel.Error);
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
            if (connectors[1].transform.parent.gameObject.activeSelf && !connectors[1].isConnected)
            {
                var exception = new ModuleException($"Maths node right input not connected to anything. You must connect a right input for this type of module: {mathsSign}", ModuleException.SeverityLevel.Error);
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
