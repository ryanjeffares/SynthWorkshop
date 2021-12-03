# Processors
This is a list of all the Modules that are considered "Processors", and how to use them.

In this section:
* [Oscillators](##Oscillators)
* [Sampling](##Sampling)
* [Enveloping](##Enveloping)
* [Filters](##Filters)
* [Maths](##Maths)

## Oscillators

### saw~
A simple saw wave, produced by entering `saw~` or `saw~ x` where `x` is some positive number into the input prompt, used as an initial frequency.

###### Inputs
Takes 1 CV input for frequency. Giving no initial frequency causes the saw wave to make no noise.

###### Outputs
Gives 2 outputs - left is Audio, right is CV. When frequency is sub-audible, the CV output can be used as a ramping LFO.

### pulse~
A simple pulse wave, produced by entering `pulse~` or `pulse~ x` where `x` is some positive number into the input prompt, used as an initial frequency.

###### Inputs
Takes 2 CV inputs: left is frequency, right is pulsewidth. Giving no initial frequency causes the pulse wave to make no noise. Pulsewidth is limited internally to be between 0 and 1, but either of these extremes will result in very little audio.

###### Outputs
Gives 2 outputs - left is Audio, right is CV. When frequency is sub-audible, the CV output can be used as a pulse LFO.

### tri~
A simple triangle wave, produced by entering `tri~` or `tri~ x` where `x` is some positive number into the input prompt, used as an initial frequency.

###### Inputs
Takes 1 CV input for frequency. Giving no initial frequency causes the triangle wave to make no noise.

###### Outputs
Gives 2 outputs - left is Audio, right is CV. When frequency is sub-audible, the CV output can be used as a triangular LFO.

### sine~
A simple sine wave, produced by entering `sine~` or `sine~ x` where `x` is some positive number into the input prompt, used as an initial frequency.

###### Inputs
Takes 1 CV input for frequency. Giving no initial frequency causes the sine wave to make no noise.

###### Outputs
Gives 2 outputs - left is Audio, right is CV. When frequency is sub-audible, the CV output can be used as a sinusoidal LFO.

## Sampling

### soundfile~
A simple sample player, produced by entering `soundfile~` to the input prompt with no additional parameters. The file browser button on the Module can then be pressed to select a sample from disk to load. Holds a single sample, and plays it back on receiving an on-trigger.

###### Inputs
**Currently not working** Takes 1 input for looping - 0 disables looping, any non 0 number enables looping.

###### Outputs
Gives 1 output, the audio of the sample.

## Enveloping

### adsr~
An ADSR (attack, decay, sustain, release) envelope with variable times and sustain level. Created by entering `adsr~` or `adsr~ a d s r` where `a`, `d`, and `r` are the attack, decay, and release times in seconds, and `s` is the sustain level (ideally 0 - 1). Giving no initial parameters produces an envelope with parameters 0.1, 0.2, 0.5, 1, respectively.

###### Inputs
Takes 6 inputs, in order left to right:
1. Audio input: the audio signal to apply the envelope to.
2. Trigger input: an on-trigger will start the attack phase, and sustain until it receives an off-trigger, starting the release phase.
3. Attack time in seconds
4. Decay time in seconds
5. Sustain level (ideally 0-1)
6. Release time in seconds

###### Outputs
Gives 1 audio output, the original audio signal with the envelope applied. This will be silence if the envelope is idle.

## Filters

### filt~
A filter that can function as a lowpass, bandpass, or highpass, and change its state during performance, with variable cutoff and resonance. Produced by entering `filt~` or `filt~ f r t` where `f` is initial cutoff frequency in Hz, `r` is initial resonance (0 or less will be limited to 1-05, upper bound is limitless), and `t` is type: 0 for lowpass, 1 for bandpass, and 2 for highpass. Giving no initial parameters produces a lowpass filter with a cutoff of 20kHz and a resonance of 0.7.

A simple filter that does not colour the sound much, but responds well to rapid changes in its parameters.

###### Inputs
Takes 4 inputs: left is audio signal to filter, middle left is CV signal for cutoff frequency in Hz, middle right is CV signal for resonance, right is CV signal for type as outlined above (non-integer values will be rounded).

###### Outputs
Gives 1 audio output, the filtered signal.

## Maths

### Standard Maths Operators
A variety of mathematical operations exist as Modules. Some operate on a single value taken from the left input, and some operate on 2 values, taken from the left and right inputs. They are made by entering the operation (as listed below), and an optional initial value, if the operation uses two values.

For example, entering `+` will create an addition module, and begin by adding 0 onto its input. Entering `+ 1` will create an addition module, and begin by adding 1 onto its input. When an input is given through a wire, this initial value is overwritten.

All maths modules can operate on CV signals. Some maths modules can operate on Audio signals as well, and are made by adding a `~` after the operator, for example `+~`. An Audio maths module can only take audio signal sfrom its left input. It can take either more audio signals **or** CV signals from its right input, but not both at the same time.

Modules that can take Audio or CV are as follows:
* `+`, `-`, `*`, `/`, `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `pow`

Modules that can only take a CV signal are as follows:
* `int`, `mod`, `mtof` (MIDI to frequency), `ftom` (frequency to MIDI)

### map
The map module will map an input CV signal from a given input range to a given output range. Created by entering `map a b c d` where `a` and `b` are the input min and max, and `c` and `d` are the output min and max.

For example, we could map the CV output of an oscillator, which would be -1 to 1, to a range usable as an LFO for, say, a filter cutoff, by using `map -1 1 1000 3000`.

###### Inputs
Left input is the incoming CV signal to be mapped, following 4 inputs are the input min, input max, output min, output max.

###### Outputs
Gives 1 output, the mapped value.