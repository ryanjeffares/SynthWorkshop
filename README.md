# SynthWorkshop
A game about building modular synths.

Note: any recent work I've done on this for MacOS has been done on an M1 chip, so if you want to run this on MacOS with an Intel CPU you should build the library with the MacOS_x86 exporter in Projucer, put the bundle in Plugins/macOs_x86/, and adjust the string in Scripts/Common/SynthWorkshopLibrary.cs line 8.

# To Do
* UI!!!!
    * Make modules smaller, more like Max
    * General make it actually look good
* Everything should be able to have multiple inputs
* Default values on the modules like in Pd/Max
* Load arrangement from json
* More maths - increment/decrement, constants like pi, two pi, e...
* Counter
* Table
* Triggers - i guess this system exists as 1 or 0, as we have in adsr already, and is just up to implementation per module?
* Repeating trigger
* Basic effects
* Delay lines
* Buffers
