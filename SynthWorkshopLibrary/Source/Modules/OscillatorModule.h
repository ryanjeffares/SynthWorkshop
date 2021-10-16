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
#include <functional>
#include "../Typedefs.h"
#include "Module.h"

class OscillatorModule : public Module
{
public:    

    OscillatorModule(OscillatorType t, std::unordered_map<int, float>& cvLookup, std::unordered_map<int, juce::AudioBuffer<float>>& audioLu, 
        int freqIdx, int pwIdx, int soundIdx, int cvOutIdx)
        : type(t), phase(0.f), frequency(20), pulseWidth(0.5f), cvParamLookup(cvLookup), frequencyInIndex(freqIdx), pulseWidthInIndex(pwIdx),
          soundOutIndex(soundIdx), cvOutIndex(cvOutIdx), audioLookup(audioLu), readyToPlay(false) {}

    ~OscillatorModule() {}

    void prepareToPlay(int samplesPerBlock, double sr) override {
        sampleRate = sr;
        samplesPerBlockExpected = samplesPerBlock;
    }

    void getNextAudioBlock(int numSamples, int numChannels) override {
        if (!readyToPlay || frequencyInIndex == -1) return;

        frequency = cvParamLookup[frequencyInIndex];
        if (pulseWidthInIndex != -1) {
            pulseWidth = cvParamLookup[pulseWidthInIndex];
            if (pulseWidth < 0.f) pulseWidth = 0.f;
            if (pulseWidth > 1.f) pulseWidth = 1.f;
        }
        float value;        
        for (auto sample = 0; sample < numSamples; sample++) {
            value = getNextSample();
            if (soundOutIndex != -1) {
                for (auto channel = 0; channel < numChannels; channel++) {
                    audioLookup[soundOutIndex].setSample(channel, sample, value);
                }
            }
        }
        cvParamLookup[cvOutIndex] = value;  // not good - this lookup should be a buffer/array so we can do per sample
    }

    const OscillatorType& getType() const {
        return type;
    }

    void setType(OscillatorType newType) {
        type = newType;
    }

private:

    float getNextSample() {
        switch (type) {
            case OscillatorType::Saw:
                output = phase;
                if (phase >= 1.f) {
                    phase -= 2.f;
                }
                phase += (1.f / (sampleRate / frequency)) * 2.f;
                return output;
            case OscillatorType::Pulse:
                if (phase >= 1.f) {
                    phase -= 1.f;
                }
                phase += (1.f / (sampleRate / frequency));
                if (phase < pulseWidth) {
                    output = -1;
                }
                if (phase > pulseWidth) {
                    output = 1;
                }
                return output;
            case OscillatorType::Sine:
                output = std::sin(phase * juce::MathConstants<float>::twoPi);
                if (phase >= 1.f) {
                    phase -= 1.f;
                }
                phase += (1.f / (sampleRate / frequency));
                return output;
            case OscillatorType::Tri:
                if (phase >= 1.f) {
                    phase -= 1.f;
                }
                phase += (1.f / (sampleRate / frequency));
                if (phase <= 0.5f) {
                    output = (phase - 0.25f) * 4;
                }
                else {
                    output = ((1.f - phase) - 0.25f) * 4;
                }
                return output;
            default: return 0;
        }
    }

    std::unordered_map<int, juce::AudioBuffer<float>>& audioLookup;
    std::unordered_map<int, float>& cvParamLookup;

    OscillatorType type;    

    double sampleRate;
    float phase, frequency, pulseWidth, output;
    const int frequencyInIndex, pulseWidthInIndex, soundOutIndex, cvOutIndex;
    int samplesPerBlockExpected;
    bool readyToPlay;

};
