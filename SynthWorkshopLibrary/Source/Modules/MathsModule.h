/*
  ==============================================================================

    MathsModule.h
    Created: 20 Jun 2021 10:45:35pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include <unordered_map>
#include <functional>
#include "../Typedefs.h"

class MathsModule
{
public:    

    MathsModule(std::unordered_map<int, float>& lookup, int leftIn, int rightIn, int output, MathsModuleType t, const std::function<float(float, float)>& func) : cvParamLookup(lookup), 
        leftInput(leftIn), rightInput(rightIn), outputIndex(output), type(t), currentFunction(func) {}

    MathsModule(std::unordered_map<int, float>& lookup, int leftIn, int rightIn, int output, int inMin, int inMax, int outMin, int outMax, MathsModuleType t) : cvParamLookup(lookup),
        leftInput(leftIn), rightInput(rightIn), outputIndex(output), minIn(inMin), minOut(outMin), maxIn(inMax), maxOut(outMax), type(t) {}

    ~MathsModule() {}

    void calculateValues() {
        if (outputIndex == -1) return;

        // checks are performed in C# so that the module will always have the necessary connections, this is just to avoid indexing the lookup at -1
        float left = leftInput == -1 ? 1 : cvParamLookup[leftInput];
        float right = rightInput == -1 ? 1 : cvParamLookup[rightInput];

        if (type == MathsModuleType::Map) {
            cvParamLookup[outputIndex] = juce::jmap<float>(left, cvParamLookup[minIn], cvParamLookup[maxIn], cvParamLookup[minOut], cvParamLookup[maxOut]);
        }
        else {
            cvParamLookup[outputIndex] = currentFunction(left, right);
        }
    }

private:
    
    std::function<float(float, float)> currentFunction;

    std::unordered_map<int, float>& cvParamLookup;
    const int leftInput, rightInput, outputIndex;
    const MathsModuleType type;

    // these are indexes for the lookup
    int minIn, maxIn, minOut, maxOut;

};