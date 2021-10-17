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

    OscillatorModule(OscillatorType t, std::unordered_map<int, std::vector<float>>& cvLookup, std::unordered_map<int, juce::AudioBuffer<float>>& audioLu, int freqIdx, int pwIdx, int soundIdx, int cvOutIdx, int id)
        : type(t), phase(0.f), frequency(20), pulseWidth(0.5f), cvParamLookup(cvLookup), frequencyInIndex(freqIdx), pulseWidthInIndex(pwIdx),
          soundOutIndex(soundIdx), cvOutIndex(cvOutIdx), audioLookup(audioLu), readyToPlay(false) 
    {
        moduleId = id;
    }

    ~OscillatorModule() = default;

    void prepareToPlay(int samplesPerBlock, double sr) override 
    {
        sampleRate = sr;
        samplesPerBlockExpected = samplesPerBlock;
    }

    void getNextAudioBlock(int numSamples, int numChannels) override 
    {
        if (!readyToPlay || frequencyInIndex == -1) return;
        
        for (auto sample = 0; sample < numSamples; sample++) 
        {
            frequency = cvParamLookup[frequencyInIndex][sample];
            if (pulseWidthInIndex != -1) 
            {
                pulseWidth = cvParamLookup[pulseWidthInIndex][sample];
                if (pulseWidth < 0.f) pulseWidth = 0.f;
                if (pulseWidth > 1.f) pulseWidth = 1.f;
            }

            float value = getNextSample();

            if (soundOutIndex != -1) 
            {
                for (auto channel = 0; channel < numChannels; channel++) 
                {
                    audioLookup[soundOutIndex].setSample(channel, sample, value);
                }
            }

            if (cvOutIndex != -1)
            {
                cvParamLookup[cvOutIndex][sample] = value;
            }
        }
    }

    void setReady(bool state) override 
    {
        readyToPlay = state;
    }

    OscillatorType getType() const 
    {
        return type;
    }

    void setType(OscillatorType newType) 
    {
        type = newType;
    }

private:

    float getNextSample() 
    {
        switch (type) 
        {
            case OscillatorType::Saw:
                output = phase;
                if (phase >= 1.f) 
                {
                    phase -= 2.f;
                }
                phase += (1.f / (sampleRate / frequency)) * 2.f;
                return output;
            case OscillatorType::Pulse:
                if (phase >= 1.f) 
                {
                    phase -= 1.f;
                }
                phase += (1.f / (sampleRate / frequency));
                if (phase < pulseWidth) 
                {
                    output = -1;
                }
                if (phase > pulseWidth) 
                {
                    output = 1;
                }
                return output;
            case OscillatorType::Sine:
                output = std::sin(phase * juce::MathConstants<float>::twoPi);
                if (phase >= 1.f) 
                {
                    phase -= 1.f;
                }
                phase += (1.f / (sampleRate / frequency));
                return output;
            case OscillatorType::Tri:
                if (phase >= 1.f) 
                {
                    phase -= 1.f;
                }
                phase += (1.f / (sampleRate / frequency));
                if (phase <= 0.5f) 
                {
                    output = (phase - 0.25f) * 4;
                }
                else 
                {
                    output = ((1.f - phase) - 0.25f) * 4;
                }
                return output;
            default: return 0;
        }
    }

    std::unordered_map<int, juce::AudioBuffer<float>>& audioLookup;
    std::unordered_map<int, std::vector<float>>& cvParamLookup;

    OscillatorType type;    

    double sampleRate;
    float phase, frequency, pulseWidth, output;
    const int frequencyInIndex, pulseWidthInIndex, soundOutIndex, cvOutIndex;
    int samplesPerBlockExpected;
    bool readyToPlay;

};
