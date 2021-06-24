/*
  ==============================================================================

    Typedefs.h
    Created: 20 Jun 2021 10:59:27pm
    Author:  ryand

  ==============================================================================
*/

#pragma once

enum class MathsModuleType {
    Plus, Minus, Multiply, Divide, Mod, Sin, Cos, Tan, Asin, Acos, Atan, Abs, Exp, Int, Map, Mtof, Ftom
};

enum class AudioCV {
    Audio, CV
};

enum class OscillatorType {
    Saw, Pulse, Sine, Tri
};

static int swFtom(float frequency) {
    return (int)(69 + (12 * std::log2f(frequency / 440.f)));
}

static float swMtof(int midiNote) {
    return std::powf(2, (midiNote - 69) / 12.f) * 440.f;
}
