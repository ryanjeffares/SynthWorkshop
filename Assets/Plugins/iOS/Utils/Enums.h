/*
  ==============================================================================

    Enums.h
    Created: 24 Nov 2021 6:11:45pm
    Author:  ryand

  ==============================================================================
*/

#pragma once

namespace SynthWorkshop
{
    namespace Enums
    {
        enum class MathsModuleType
        {
            Plus, Minus, Multiply, Divide, Sin, Cos, Tan, Asin, Acos, Atan, Abs, Exp, Int, Mod, Map, Mtof, Ftom
        };

        enum class AudioCV
        {
            Audio, CV, Trigger
        };

        enum class OscillatorType
        {
            Sine, Saw, Pulse, Tri
        };
    }
}