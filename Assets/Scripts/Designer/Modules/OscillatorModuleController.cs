using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.UI;

public enum OscillatorType
{
    Sine, Saw, Pulse, Tri
}

public class OscillatorModuleController : ModuleParent
{
    [SerializeField] private List<Sprite> waveformSprites;
    [SerializeField] private Image icon;

    private OscillatorType _type;
    public OscillatorType Type
    {
        get => _type;
        set
        {
            _type = value;
            icon.sprite = waveformSprites[(int)Type];
            GetComponent<Image>().color = Type switch
            {
                OscillatorType.Sine => "#AEBDFF".ColourFromHex(),
                OscillatorType.Saw => "#FFAEB3".ColourFromHex(),
                OscillatorType.Pulse => "#F9AEFF".ColourFromHex(),
                OscillatorType.Tri => "#AEFFB2".ColourFromHex(),
                _ => Color.white,
            };
            nameText.text = Type.ToString();
            if (Type == OscillatorType.Pulse)
            {
                connectors[4].transform.parent.gameObject.SetActive(true);
                connectors[3].transform.parent.localPosition = new Vector3(0, 75);
            }
        }
    }

    protected override void ChildAwake()
    {
        moduleType = ModuleType.OscillatorModule;        
    }

    public void SetType(OscillatorType t)
    {
        Type = t;
    }

    public Dictionary<string, bool> GetConnectorStates()
    {
        return new Dictionary<string, bool>
        {
            {"SoundOutTo", connectors[0].isConnected},
            {"CVOutTo", connectors[1].isConnected},
            {"FreqInputFrom", connectors[2].isConnected},
            {"FMInputFrom", connectors[3].isConnected},
            {"PWInputFrom", connectors[4].isConnected}
        };
    }

    public void CreateJsonEntry(Dictionary<string, object> jsonDict, Dictionary<ModuleConnectorController, int> outputLookup)
    {
        var propertyNames = new[] { "SoundOutputTo", "CVOutputTo", "FreqInputFrom", "FMInputFrom", "PWInputFrom" };
        for(int i = 0; i < connectors.Count; i++)
        {
            if (!connectors[i].isConnected) continue;
            if(connectors[i].inputOutput == InputOutput.Output)
            {
                jsonDict.Add(propertyNames[i], outputLookup[connectors[i]]);
            }
            else
            {
                jsonDict.Add(propertyNames[i], outputLookup[connectors[i].sourceConnector]);
            }
        }
    }
}
