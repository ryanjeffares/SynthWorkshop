# SynthWorkshop
A game about building modular synths.

Note: any recent work I've done on this for MacOS has been done on an M1 chip, so if you want to run this on MacOS with an Intel CPU you should build the library with the MacOS_x86 exporter in Projucer, put the bundle in Plugins/macOs_x86/, and adjust the string in Scripts/Common/SynthWorkshopLibrary.cs line 8.

# To Do
* VERY IMPORTANT: Triggers - instead of using 0/1 use a function callback, toggles and buttons just dont work rn
* Load arrangement from json - finish this when line connectors are in
* UI!!!!
    * Make modules smaller, more like Max
    * General make it actually look good
* Everything should be able to have multiple inputs - think its only ADSR that doesnt have that rn
* Default values on the modules like in Pd/Max
* More maths - increment/decrement, constants like pi, two pi, e...
* Counter
* Table
* Repeating trigger
* Basic effects
* Delay lines
* Buffers

# Overhaul to make live editing possible TO DO:
* Oscillators, knobs, audio outputs basically done - possible bug when deleting
* So just need implementation for the rest of the module types
* Deleting a single wire
