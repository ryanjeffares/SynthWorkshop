using LogicUI.FancyTextRendering;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using UnityEngine.UI;

public class DocumentationController : MonoBehaviour
{
    [SerializeField] private MarkdownRenderer markdownRenderer;
    [SerializeField] private Button general, processors, io, triggers, interaction, close;

    private void Awake()
    {
        var path = Path.Combine(Application.streamingAssetsPath, "Documentation", "General Usage.md");
        markdownRenderer.Source = File.ReadAllText(path);

        general.onClick.AddListener(() =>
        {
            var path = Path.Combine(Application.streamingAssetsPath, "Documentation", "General Usage.md");
            markdownRenderer.Source = File.ReadAllText(path);
        });

        processors.onClick.AddListener(() =>
        {
            var path = Path.Combine(Application.streamingAssetsPath, "Documentation", "Processors.md");
            markdownRenderer.Source = File.ReadAllText(path);
        });

        io.onClick.AddListener(() =>
        {
            var path = Path.Combine(Application.streamingAssetsPath, "Documentation", "IO.md");
            markdownRenderer.Source = File.ReadAllText(path);
        });

        triggers.onClick.AddListener(() =>
        {
            var path = Path.Combine(Application.streamingAssetsPath, "Documentation", "Triggers.md");
            markdownRenderer.Source = File.ReadAllText(path);
        });

        interaction.onClick.AddListener(() =>
        {
            var path = Path.Combine(Application.streamingAssetsPath, "Documentation", "Interaction.md");
            markdownRenderer.Source = File.ReadAllText(path);
        });

        close.onClick.AddListener(() => Destroy(gameObject));
    }
}
