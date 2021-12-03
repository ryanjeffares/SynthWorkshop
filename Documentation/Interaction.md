# Interaction
This is a list of all the Modules considered "Interaction", and how to use them.

### knob
A radial slider, with changeable min, max, skew factor, and integer rounding, usable on the designer. Gives a single CV output, the current value of the slider.

A knob can be created in 3 ways:
1. `knob` gives a slider with a min of 0 and a max of 1, and an initial value of 0.5, with no skewing and no rounding.
2. `knob a b c` where `a` is the minimum, `b` is the maximum, and `c` is the initial value, with no skewing or rounding.
3. `knob a b c d e` where `a`, `b`, and `c` are as above, `d` is a skew factor, and `e` is integer rounding (any value greater than 0 will activate integer rounding). Skew values greater than 1 give higher resolution in the lower end of the range, and values less than 1 give higher resolution in the upper range.