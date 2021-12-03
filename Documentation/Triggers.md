# Triggers
This is a list of all the Modules considered "Triggers", and how to use them.

In this section:
1. [bang](###bang)
2. [toggle](###toggle)
3. [bdelay](###bdelay)


### bang
A button, pressable in the Designer, that gives a single instantaneous on-trigger message. Can be triggered by other bangs or triggers through its input, but not itself (ie connecting a bang's output to its own input is not allowed, unless with a bang delay (see below) in between).

###### Special Interactions
Despite always giving an on-trigger, a bang will **always** flip the state of a toggle (see below). Similarly, a toggle will always trigger a bang when its state changes.

### toggle
A switch, pressable in the Designer, that gives an instantaneous on-trigger message when activated, and off-trigger when deactivated. Similarly to the bang above, cannot be directly connected to its own input.

###### Special Interactions
As described above, a toggle will always trigger a bang connected to its output when its state changes. A bang connected to the toggle's input will always flip the state of the toggle.

### bdelay
"Bang delay", that simply delays a trigger message, delivered through its left input, and passed on out of its single trigger output. Must be given an initial delay time in seconds like `bdelay t` where `t` is time in seconds, and can take a CV input through its right input for delay time.

Can take input from bangs and toggles. When a bang delay sends its output, it sends a reference to the original sender, so any special interactions between bangs and toggles are respected.