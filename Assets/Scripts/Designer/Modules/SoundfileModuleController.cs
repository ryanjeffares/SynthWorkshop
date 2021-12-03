using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;
using UnityEngine.UI;
using SimpleFileBrowser;

public class SoundfileModuleController : ModuleParent
{
    [SerializeField] private Button openFileButton;

    protected override void ChildAwake()
    {
        FileBrowser.SetFilters(true, "wav", "mp3", "ogg", "aiff", "flac", "m4a");
        openFileButton.onClick.AddListener(() => StartCoroutine(BrowseForFile()));
    }

    public void SetOutputIndex(int idx)
    {
        connectors[2].SetOutputIndex(idx);
    }

    private IEnumerator BrowseForFile()
    {
        yield return FileBrowser.WaitForLoadDialog(FileBrowser.PickMode.Files);

        if (FileBrowser.Success)
        {

            if (SynthWorkshopLibrary.SetSoundfileSample(GlobalIndex, FileBrowser.Result[0]))
            {
                Debug.Log("Success: " + FileBrowser.Result[0]);            
            }
        }
    }

    public override List<ModuleException> CheckErrors()
    {
        return new List<ModuleException>();
    }
}
