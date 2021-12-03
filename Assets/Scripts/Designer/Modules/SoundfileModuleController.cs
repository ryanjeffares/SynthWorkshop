using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;
using UnityEngine.UI;

public class SoundfileModuleController : ModuleParent
{
    [SerializeField] private Button openFileButton;

    protected override void ChildAwake()
    {
        openFileButton.onClick.AddListener(BrowseForFile);
    }

    public void SetOutputIndex(int idx)
    {
        connectors[2].SetOutputIndex(idx);
    }

    private void BrowseForFile()
    {
        var path = EditorUtility.OpenFilePanelWithFilters("Select sound file", Application.dataPath + "/Samples", new[] { "Audio files", "wav,mp3,ogg" });
        if (SynthWorkshopLibrary.SetSoundfileSample(GlobalIndex, path))
        {
            Debug.Log("Success: " + path);
        }
    }

    public override List<ModuleException> CheckErrors()
    {
        return new List<ModuleException>();
    }
}
