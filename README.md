# Synth Workshop
A visual medium for creating music inspired by Pure Data and Max/MSP, with a focus on intuitiveness.

---

## Running
You can run Synth Workshop from Unity. I work with Unity version 2020.3.0f1, so that version or newer should be fine. You could also build it, which you must do for iOS.

Synth Workshop runs on Windows, Mac, and iOS. The audio is handled by a plugin written in C++ using the JUCE framework, which can be found in the SynthWorkshopLibrary directory in the root of the project. The plugin is placed in the platform specific folder in Assets/Plugins - I may not have an up-to-date build there for your platform, so I would install JUCE if you don't already have it and build the plugin using Projucer (`SynthWorkshopLibrary.jucer` for Windows, `SynthWorkshopBundle` for Mac, and `SynthWorkshopStatic` for iOS*), and place the build in the right folder.

## Documentation
I keep documentation for the node-based visual scripting "language" in the Documentation folder.

## Contributing
The code is **not** documented and kind of a rats' nest, but be my guest!

---

* When building for iOS, as well as the static library, all of the C++ header files must be in the Plugins/iOS folder too. To ensure they are up to date, you can run `copy_headers.py` and give it the path to the Source folder in the JUCE project like this: `python3 copy_headers.py ~/path_to_unity_project/SynthWorkshopLibrary/Source` and for the love of God, don't put a trailing / on that path, the Python code is terrible.