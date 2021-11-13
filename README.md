# SynthWorkshop
A game about building modular synths.

Note: any recent work I've done on this for MacOS has been done on an M1 chip, so if you want to run this on MacOS with an Intel CPU you should build the library with the MacOS_x86 exporter in Projucer, put the bundle in Plugins/macOs_x86/, and adjust the string in Scripts/Common/SynthWorkshopLibrary.cs line 8.

# To Do
* VERY IMPORTANT: Triggers - bangs and toggles
* Bug with removing modules cutting off other modules output, some kind of indexing error: track this down....
* Load arrangement from json - finish this when line connectors are in
* UI!!!!
	* font??
	* General make it actually look good
* Everything should be able to have multiple inputs - map param inputs dont have that currently
* More maths - increment/decrement, constants like pi, two pi, e...
* Counter
* Table
* Repeating trigger
* Basic effects
* Delay lines
* Buffers