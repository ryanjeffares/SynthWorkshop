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

class OscillatorModule
{
public:

    enum OscillatorType {
        Saw, Pulse, Sine, Tri
    };

    OscillatorModule(OscillatorType t, std::unordered_map<int, float>& cvLookup, std::unordered_map<int, juce::AudioBuffer<float>>& audioLu, 
        int freqIdx, int pwIdx, int fmInIdx, int soundIdx, int cvOutIdx)
        : type(t), phase(0), frequency(20), pulseWidth(0.5f), cvParamLookup(cvLookup), frequencyInIndex(freqIdx), pulseWidthInIndex(pwIdx),
          fmInIndex(fmInIdx), soundOutIndex(soundIdx), cvOutIndex(cvOutIdx), audioLookup(audioLu), readyToPlay(false) {}

    ~OscillatorModule() {}

    void prepareToPlay(int samplesPerBlock, double sr) {
        sampleRate = sr;
        samplesPerBlockExpected = samplesPerBlock;
    }

    void getNextAudioBlock(int numSamples, int numChannels) {
        if (!readyToPlay || frequencyInIndex == -1) return;
        frequency = cvParamLookup[frequencyInIndex];
        if (frequency <= 1) {
            frequency = 1;
        }
        pulseWidth = cvParamLookup[pulseWidthInIndex];
        if (pulseWidth < 0) pulseWidth = 0;
        if (pulseWidth > 1) pulseWidth = 1;
        for (auto sample = 0; sample < numSamples; sample++) {
            auto value = processSample();
            for (auto channel = 0; channel < numChannels; channel++) {
                if (soundOutIndex != -1) {
                    if (fmInIndex != -1) {
                        audioLookup[soundOutIndex].addSample(channel, sample, value * audioLookup[fmInIndex].getSample(channel, sample));
                    }
                    else {
                        audioLookup[soundOutIndex].addSample(channel, sample, value);
                    }
                }
            }
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
    int frequencyInIndex, pulseWidthInIndex, fmInIndex, soundOutIndex, cvOutIndex;
    int samplesPerBlockExpected;
    bool readyToPlay;

};