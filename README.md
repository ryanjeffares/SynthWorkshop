# SynthWorkshop
A game about building modular synths.

Note: any recent work I've done on this for MacOS has been done on an M1 chip, so if you want to run this on MacOS with an Intel CPU you should build the library with the MacOS_x86 exporter in Projucer, put the bundle in Plugins/macOs_x86/, and adjust the string in Scripts/Common/SynthWorkshopLibrary.cs line 8.

# To Do
* Wires are un-fucked but see if we can fix the position jumping around when you move to target module
* Zoom with two fingers on iOS
* Message box
	* I'm thinking a message box can only give an output to a number box, number boxes have a special case to accept values from message and hold only that value
* File browser
* Grouping modules together into submodules
* Detect clicks on wires...
* Update `_currentArrangement` properly so we can use it to save arrangements
* Use a key other than `~` to denote audio, as it is a pain to type on iOS
	* maybe have a dropdown of suggestions, no other characters really work i think
	* maybe its not that bad
* Load arrangement from json
* Hunt for bugs - seems pretty robust now but be ever vigilant
* UI!!!!
	* font??
	* General make it actually look good
* More maths - increment/decrement, constants like pi, two pi, e...
* Comments
* Better sampler
	* Current one is intended to be simple, but lets improve it, get looping in
	* Should have pitch shifting sampler, multi sampler...
	* Waveform display!
* Counter
* Table
* Repeating trigger/metronome
* Basic effects
* Delay lines
* Buffers