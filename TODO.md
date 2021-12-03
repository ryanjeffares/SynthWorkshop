# To Do
## UI/Interaction
* Wires are un-fucked but see if we can fix the position jumping around when you move to target module
* Zoom with two fingers on iOS
* Grouping modules together into submodules
* Detect clicks on wires...
* font??
* General make it actually look good
* Use a key other than `~` to denote audio, as it is a pain to type on iOS
	* maybe have a dropdown of suggestions, no other characters really work i think
	* maybe its not that bad

## Workflow
* Message box
	* I'm thinking a message box can only give an output to a number box, number boxes have a special case to accept values from message and hold only that value
* Update `_currentArrangement` properly so we can use it to save arrangements
* Load arrangement from json
* File browser that will run in a standalone

## Technical
* Hunt for bugs - seems pretty robust now but be ever vigilant
	
## Modules
* Add initial pulsewidth parameter to pulse wave
* Phase reset trigger input to oscillators
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