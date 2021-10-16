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
#include "Module.h"

class MathsModule : public Module
{
public:    

    // used for all other modules
    MathsModule(std::unordered_map<int, float>& lookup, std::vector<int> leftIn, std::vector<int> rightIn, int output, MathsModuleType t, const std::function<float(float, float)>& func) : cvParamLookup(lookup),
        leftInputs(leftIn), rightInputs(rightIn), outputIndex(output), type(t), currentFunction(func) {}

    // used only for Map module
    MathsModule(std::unordered_map<int, float>& lookup, std::vector<int> leftIn, std::vector<int> rightIn, int output, int inMin, int inMax, int outMin, int outMax, MathsModuleType t) : cvParamLookup(lookup),
        leftInputs(leftIn), rightInputs(rightIn), outputIndex(output), minIn(inMin), minOut(outMin), maxIn(inMax), maxOut(outMax), type(t) {}

    ~MathsModule() {}

    void calculateValues() {
        if (outputIndex == -1 || !readyToPlay) return;
        
        float left = 0.f;
        float right = 0.f;
        
        for (auto i : leftInputs) {
            left += cvParamLookup[i];
        }
        for (auto i : rightInputs) {
            right += cvParamLookup[i];
        }
        
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
    std::vector<int> leftInputs, rightInputs;
    const int outputIndex;
    const MathsModuleType type;

    // these are indexes for the lookup
    int minIn, maxIn, minOut, maxOut;

};
