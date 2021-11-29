# SynthWorkshop
A game about building modular synths.

Note: any recent work I've done on this for MacOS has been done on an M1 chip, so if you want to run this on MacOS with an Intel CPU you should build the library with the MacOS_x86 exporter in Projucer, put the bundle in Plugins/macOs_x86/, and adjust the string in Scripts/Common/SynthWorkshopLibrary.cs line 8.

# To Do
* Bangs
	* A bang should only send one message
	* A bang should always switch the state of a toggle
	* For all other triggerables, a bang should **ONLY** give it its "on" message
	* Make toggles/bangs react in UI when triggered from another bang/toggle
* Wires are un-fucked but see if we can fix the position jumping around when you move to target module
* Framerate feels slow on iOS
* Number box destroys performane in debug
* Message box
* Detect clicks on wires...
* Work on double clicking - using `eventData.clickCount` wasn't working on iOS, but the new system is detecting a bunch of double presses when it shouldn't
* Use a key other than `~` to denote audio, as it is a pain to type on iOS
	* maybe have a dropdown of suggestions, no other characters really work i think
* Load arrangement from json - finish this when line connectors are in
* Hunt for bugs - seems pretty robust now but be ever vigilant
* Crash on shutdown i suspect because audio thread is modifying vectors while unity thread tries to remove connections
* UI!!!!
	* font??
	* General make it actually look good
* More maths - increment/decrement, constants like pi, two pi, e...
* Counter
* Table
* Repeating trigger
* Basic effects
* Delay lines
* Buffers