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

class AudioMathsModule
{
public:

    AudioMathsModule(std::unordered_map<int, float>& cvLookup, std::unordered_map<int, juce::AudioBuffer<float>>& audioLu, std::vector<int> leftIn, int rightIn, int output, AudioCV acv, const std::function<float(float, float)>& func)
        : cvParamLookup(cvLookup), audioLookup(audioLu), leftInputs(leftIn), rightInput(rightIn), outputIndex(output), audioCvIncoming(acv), currentFunction(func) {}

    ~AudioMathsModule() {}

    void prepareToPlay(int spbe, double sr) {
        sampleRate = sr;
        samplesPerBlockExpected = spbe;
    }

    void releaseResources() {

    }

    void getNextAudioBlock(int numChannels, int numSamples) {
        if (!readyToPlay || outputIndex == -1) return;
        auto write = audioLookup[outputIndex].getArrayOfWritePointers();
        for (int sample = 0; sample < numSamples; sample++) {
            for (int channel = 0; channel < numChannels; channel++) {
                // checks are performed in C# so that the module will always have the necessary connections, this is just to avoid indexing the lookup at -1
                
                float affectingVal = rightInput == -1 ? 1 : (audioCvIncoming == AudioCV::Audio ? audioLookup[rightInput].getSample(channel, sample) : cvParamLookup[rightInput]);
                
                float inputSampleVal = 0.f;
                for (auto l : leftInputs) {
                    inputSampleVal += audioLookup[l].getSample(channel, sample);
                }
                
                write[channel][sample] = currentFunction(inputSampleVal, affectingVal);
            }
        }
    }

    void setReady(bool state) {
        readyToPlay = state;
    }

private:

    std::unordered_map<int, float>& cvParamLookup;
    std::unordered_map<int, juce::AudioBuffer<float>>& audioLookup;

    std::function<float(float, float)> currentFunction;

    std::vector<int> leftInputs;
    
    const int rightInput;
    const int outputIndex;
    const AudioCV audioCvIncoming;

    double sampleRate;
    int samplesPerBlockExpected;

    bool readyToPlay = false;
};
