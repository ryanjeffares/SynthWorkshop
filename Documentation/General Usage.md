# General Usage
* There is one scene in the Unity project which is the **Designer**. In the future there will be more scenes and features, but for now there is just the one scene where you can make an **Arrangement**.
* An Arrangement is a collection of **Modules**, connected together to make sound/music.

## Modules
* When in the Designer, you can double click anywhere on the background to create a new Module, and double tap and existing Module to destroy it.
* After double clicking on empty space, it will create a text input box for you to type in the name of the Module you want and any initial parameters it may take, if any. You can double click this again to cancel, or type `"delete"` on iOS and submit. Press enter (or submit on iOS) to create the Module.
* If there was a problem with your input, the text will turn red, or else the Module will be created and the text box will be destroyed.
* You can drag from an output of a Module to create a wire and connect it to an input of another Module, if the connection is allowed.
* All inputs can take multiple inputs, and all outputs can output to multiple places.
* If an input is Audio or CV (see below), its inputs will be summed together. If an input is Trigger, a trigger message can be received individually from any of its inputs. Certain triggerable Modules have behaviours specific to certain triggers, and this is respected when there are multiple inputs.
* Currently the only way to remove a wire is to double click on an output/input, and click the connection you want to remove from the dropdown. In future I hope to make it so you can double click the wire itself to destroy it, but I'm not sure if I can detect clicks on the wires with the way they are being rendered currently.
* Audio processing is on by default, but you can pause and resume processing using the Stop/Resume button in the top right.
* You can destroy every currently active Module by pressing the Clear button in the top right.

## Signal Types
* There are 3 main signal types (currently): **Audio**, **CV**, and **Trigger**.
* Audio carries an audio signal (duh) that can be processed and outputted to the speakers.
* CV carries numerical values that are used for parameters in many of the Modules, and for mathematical operations on both other CV signals and Audio signals.
* Triggers are instantaneous messages that trigger an event or toggle a state on or off. Holds an "on" or "off" state.
* Inputs on Modules usually take a single type of signal, and outputs only output a single type. The wire drawn from an output will have a different colour depending on the type: Audio is black, CV is grey, and Trigger is blue.

## Interaction
* The size of your arrangement is theoretically unlimited.
* You can drag on the empty space to move the view, this has no limits.
* You can zoom with the mouse wheel or on screen slider the zoom in our out, this is limited to be between 0.01x and 3x.
* You can press the Hide UI button in the top right to hide all UI elements (except for that button).