# SynthWorkshop
A game about building modular synths.

Note: any recent work I've done on this for MacOS has been done on an M1 chip, so if you want to run this on MacOS with an Intel CPU you should build the library with the MacOS_x86 exporter in Projucer, put the bundle in Plugins/macOs_x86/, and adjust the string in Scripts/Common/SynthWorkshopLibrary.cs line 8.

# To Do
* Wires are un-fucked but see if we can fix the position jumping around when you move to target module
* Number box destroys performance in debug
* Zoom with two fingers on iOS/scroll on desktop
	* Move view when zoomed
* Message box
	* I'm thinking a message box can only give an output to a number box, number boxes have a special case to accept values from message and hold only that value
	* Requires fixing the above issue with number boxes killing the performance in debug lol
* Grouping modules together into submodules
* Detect clicks on wires...
* Use a key other than `~` to denote audio, as it is a pain to type on iOS
	* maybe have a dropdown of suggestions, no other characters really work i think
	* maybe its not that bad
* Load arrangement from json
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