/*
  ==============================================================================

    AudioMathsModule.h
    Created: 20 Jun 2021 10:57:36pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include <unordered_map>
#include <JuceHeader.h>
#include "../Typedefs.h"

class AudioMathsModule
{
public:

    AudioMathsModule(std::unordered_map<int, float>& cvLookup, std::unordered_map<int, juce::AudioBuffer<float>>& audioLu, int leftIn, int rightIn, int output, MathsModuleType t, AudioCV acv)
        : cvParamLookup(cvLookup), audioLookup(audioLu), leftInput(leftIn), rightInput(rightIn), outputIndex(output), type(t), audioCvIncoming(acv) {}

    ~AudioMathsModule() {}

    void prepareToPlay(int spbe, double sr) {
        sampleRate = sr;
        samplesPerBlockExpected = spbe;
    }

    void releaseResources() {

    }

    void getNextAudioBlock(int numChannels, int numSamples) {
        if (!readyToPlay || outputIndex == -1) return;
        for (int sample = 0; sample < numSamples; sample++) {
            for (int channel = 0; channel < numChannels; channel++) {
                float inputSampleVal = leftInput == -1 ? 1 : audioLookup[leftInput].getSample(channel, sample);
                float affectingVal = rightInput == -1 ? 1 : (audioCvIncoming == AudioCV::Audio ? audioLookup[rightInput].getSample(channel, sample) : cvParamLookup[rightInput]);
                switch (type) {
                    case MathsModuleType::Plus:
                        audioLookup[outputIndex].setSample(channel, sample, inputSampleVal + affectingVal);
                        break;
                    case MathsModuleType::Minus:
                        audioLookup[outputIndex].setSample(channel, sample, inputSampleVal - affectingVal);
                        break;
                    case MathsModuleType::Multiply:
                        audioLookup[outputIndex].setSample(channel, sample, inputSampleVal * affectingVal);
                        break;
                    case MathsModuleType::Divide:
                        audioLookup[outputIndex].setSample(channel, sample, inputSampleVal / affectingVal);
                        break;
                }
            }
        }
    }

    void setReady(bool state) {
        readyToPlay = state;
    }

private:

    std::unordered_map<int, float>& cvParamLookup;
    std::unordered_map<int, juce::AudioBuffer<float>>& audioLookup;

    const int leftInput, rightInput, outputIndex;
    const MathsModuleType type;
    const AudioCV audioCvIncoming;

    double sampleRate;
    int samplesPerBlockExpected;

    bool readyToPlay = false;
};