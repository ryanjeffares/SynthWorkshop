/*
  ==============================================================================

    OscillatorModule.h
    Created: 12 Jun 2021 9:05:03pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <unordered_map>
#include "../Typedefs.h"

class OscillatorModule
{
public:    

    OscillatorModule(OscillatorType t, std::unordered_map<int, float>& cvLookup, std::unordered_map<int, juce::AudioBuffer<float>>& audioLu, 
        int freqIdx, int pwIdx, int rmInIdx, int soundIdx, int cvOutIdx)
        : type(t), phase(0), frequency(20), pulseWidth(0.5f), cvParamLookup(cvLookup), frequencyInIndex(freqIdx), pulseWidthInIndex(pwIdx),
          rmInIndex(rmInIdx), soundOutIndex(soundIdx), cvOutIndex(cvOutIdx), audioLookup(audioLu), readyToPlay(false) {}

    ~OscillatorModule() {}

    void prepareToPlay(int samplesPerBlock, double sr) {
        sampleRate = sr;
        samplesPerBlockExpected = samplesPerBlock;
    }

    void getNextAudioBlock(int numSamples, int numChannels) {
        if (!readyToPlay || frequencyInIndex == -1) return;
        frequency = cvParamLookup[frequencyInIndex];
        if (frequency <= 0 ) {
            frequency = 1e-5;
        }
        pulseWidth = cvParamLookup[pulseWidthInIndex];
        if (pulseWidth < 0) pulseWidth = 0;
        if (pulseWidth > 1) pulseWidth = 1;
        for (auto sample = 0; sample < numSamples; sample++) {
            auto value = processSample();
            for (auto channel = 0; channel < numChannels; channel++) {
                if (soundOutIndex == -1) continue;
                if (rmInIndex != -1) {
                    audioLookup[soundOutIndex].setSample(channel, sample, value * audioLookup[rmInIndex].getSample(channel, sample));
                }
                else {
                    audioLookup[soundOutIndex].setSample(channel, sample, value);
                }
            }
            cvParamLookup[cvOutIndex] = value;  // not good - this lookup should be a buffer/array so we can do per sample
        }
    }

    void releaseResources() {

    }

    OscillatorType getType() {
        return type;
    }

    void setType(OscillatorType newType) {
        type = newType;
    }

    void setReady(bool state) {
        readyToPlay = state;
    }

private:

    float processSample() {
        switch (type) {
            case OscillatorType::Saw: {
                if (phase >= 1) {
                    phase = -1;
                }
                phase += (1.f / (sampleRate / frequency)) * 2;
                return phase;
            }
            case OscillatorType::Pulse: {
                if (phase >= 1) {
                    phase = 0;
                }
                phase += (1.f / (sampleRate / frequency));
                return phase > pulseWidth ? 1 : -1;
            }
            case OscillatorType::Sine: {
                if (phase >= 1) {
                    phase = 0;
                }
                phase += (1.f / (sampleRate / frequency));
                return std::sin(phase * juce::MathConstants<float>::twoPi);
            }
            case OscillatorType::Tri: {
                if (phase >= 1) {
                    phase = 0;
                }
                phase += (1.f / (sampleRate / frequency));
                return phase <= 0.5f ? (phase - 0.25f) * 4 : ((1.0f - phase) - 0.25) * 4;
            }
            default: return 0;
        }
    }

    std::unordered_map<int, juce::AudioBuffer<float>>& audioLookup;
    std::unordered_map<int, float>& cvParamLookup;
    OscillatorType type;
    double sampleRate;
    float phase, frequency, pulseWidth;
    const int frequencyInIndex, pulseWidthInIndex, rmInIndex, soundOutIndex, cvOutIndex;
    int samplesPerBlockExpected;
    bool readyToPlay;

};