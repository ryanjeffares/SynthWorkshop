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
    MathsModule(std::unordered_map<int, std::vector<float>>& lookup, std::vector<int> leftIn, std::vector<int> rightIn, int output, MathsModuleType t, const std::function<float(float, float)>& func, int id)
        : cvParamLookup(lookup), leftInputs(leftIn), rightInputs(rightIn), outputIndex(output), type(t), currentFunction(func) 
    {
        moduleId = id;
    }

    // used only for Map module
    MathsModule(std::unordered_map<int, std::vector<float>>& lookup, std::vector<int> leftIn, std::vector<int> rightIn, int output, int inMin, int inMax, int outMin, int outMax, MathsModuleType t, int id)
        : cvParamLookup(lookup), leftInputs(leftIn), rightInputs(rightIn), outputIndex(output), minIn(inMin), minOut(outMin), maxIn(inMax), maxOut(outMax), type(t) 
    {
        moduleId = id;
    }

    ~MathsModule() = default;


    void getNextAudioBlock(int numSamples, int numChannels) override
    {
        if (outputIndex == -1 || !readyToPlay) return;

        for (auto sample = 0; sample < numSamples; sample++)
        {
            float left = 0.f;
            float right = 0.f;

            for (auto i : leftInputs)
            {
                left += cvParamLookup[i][sample];
            }

            for (auto i : rightInputs)
            {
                right += cvParamLookup[i][sample];
            }

            if (type == MathsModuleType::Map)
            {
                cvParamLookup[outputIndex][sample] = juce::jmap<float>(
                    left, cvParamLookup[minIn][sample], cvParamLookup[maxIn][sample], cvParamLookup[minOut][sample], cvParamLookup[maxOut][sample]
                );
            }
            else
            {
                cvParamLookup[outputIndex][sample] = currentFunction(left, right);
            }
        }        
    }

    void setReady(bool state) override 
    {
        readyToPlay = state;
    }

private:
    
    std::function<float(float, float)> currentFunction;

    std::unordered_map<int, std::vector<float>>& cvParamLookup;
    std::vector<int> leftInputs, rightInputs;
    const int outputIndex;
    const MathsModuleType type;

    // these are indexes for the lookup
    int minIn, maxIn, minOut, maxOut;

};
