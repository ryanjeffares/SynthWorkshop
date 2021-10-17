/*
  ==============================================================================

    AudioMathsModule.h
    Created: 20 Jun 2021 10:57:36pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <unordered_map>
#include <functional>
#include "../Typedefs.h"
#include "Module.h"

class AudioMathsModule : public Module
{
public:

    AudioMathsModule(std::unordered_map<int, std::vector<float>>& cvLookup, std::unordered_map<int, juce::AudioBuffer<float>>& audioLu, std::vector<int> leftIn, std::vector<int> rightIns, int output, AudioCV acv, const std::function<float(float, float)>& func)
        : cvParamLookup(cvLookup), audioLookup(audioLu), leftInputs(leftIn), rightInputs(rightIns), outputIndex(output), audioCvIncoming(acv), currentFunction(func) {}

    ~AudioMathsModule() = default;

    void prepareToPlay(int spbe, double sr) override 
    {
        sampleRate = sr;
        samplesPerBlockExpected = spbe;
    }

    void getNextAudioBlock(int numSamples, int numChannels) override 
    {
        if (!readyToPlay || outputIndex == -1) return;
        auto write = audioLookup[outputIndex].getArrayOfWritePointers();
        for (int sample = 0; sample < numSamples; sample++) 
        {
            for (int channel = 0; channel < numChannels; channel++) 
            {
                // checks are performed in C# so that the module will always have the necessary connections                
                float affectingVal = 0.f;
                for (auto r : rightInputs)
                {
                    affectingVal += audioCvIncoming == AudioCV::Audio ? audioLookup[r].getSample(channel, sample) : cvParamLookup[r][sample];
                }
                
                float inputSampleVal = 0.f;
                for (auto l : leftInputs) 
                {
                    inputSampleVal += audioLookup[l].getSample(channel, sample);
                }
                
                write[channel][sample] = currentFunction(inputSampleVal, affectingVal);
            }
        }
    }

    void setReady(bool state) override 
    {
        readyToPlay = state;
    }

private:

    std::unordered_map<int, std::vector<float>>& cvParamLookup;
    std::unordered_map<int, juce::AudioBuffer<float>>& audioLookup;

    std::function<float(float, float)> currentFunction;

    std::vector<int> leftInputs;
    std::vector<int> rightInputs;
    
    const int outputIndex;
    const AudioCV audioCvIncoming;

    double sampleRate;
    int samplesPerBlockExpected;

    bool readyToPlay = false;
};
