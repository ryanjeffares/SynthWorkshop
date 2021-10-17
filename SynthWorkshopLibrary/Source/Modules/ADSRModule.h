/*
  ==============================================================================

    ADSRModule.h
    Created: 23 Jun 2021 9:28:26pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <vector>
#include <unordered_map>
#include "Module.h"

class ADSRModule : public Module
{
public:

    ADSRModule(std::unordered_map<int, juce::AudioBuffer<float>>& audioLu, std::unordered_map<int, std::vector<float>>& cvLookup, std::vector<int> inputIndexes, int outputIndex, int numCh,
        int a, int d, int s, int r, int t) 
        : audioLookup(audioLu), cvParamLookup(cvLookup), audioOutputIndex(outputIndex), numChannels(numCh), attackIndex(a), decayIndex(d), sustainIndex(s), releaseIndex(r), triggerInput(t) 
    {
        audioInputIndexes = inputIndexes;
    }

    void prepareToPlay(int spbe, double sr) override 
    {
        samplesPerBlockExpected = spbe;
        sampleRate = sr;
        adsr.setSampleRate(sampleRate);
    }

    void getNextAudioBlock(int numSamples, int numChannels) override 
    {
        if (!readyToPlay || audioOutputIndex == -1) return;
        
        updateEnvelope();
        audioLookup[audioOutputIndex].clear();
        auto write = audioLookup[audioOutputIndex].getArrayOfWritePointers();
        
        for (int sample = 0; sample < numSamples; sample++) 
        {            
            float value = adsr.getNextSample();
            
            for (int channel = 0; channel < numChannels; channel++) 
            {
                for (const int& i : audioInputIndexes) {
                    write[channel][sample] += (audioLookup[i].getSample(channel, sample) * value);
                }
            }
        }
    }

    void setReady(bool state) override 
    {
        readyToPlay = state;
    }

private:
    
    void updateEnvelope()
    {
        currentParameters.attack = cvParamLookup[attackIndex][0];
        currentParameters.decay = cvParamLookup[decayIndex][0];
        currentParameters.sustain = cvParamLookup[sustainIndex][0];
        currentParameters.release = cvParamLookup[releaseIndex][0];

        adsr.setParameters(currentParameters);

        if (triggerInput != -1 && noteOnState != cvParamLookup[triggerInput][0]) 
        {
            noteOnState = cvParamLookup[triggerInput][0];
            if (noteOnState) 
            { 
                adsr.noteOn();                
            }
            else 
            { 
                adsr.noteOff();
            }
        }
    }

    std::unordered_map<int, juce::AudioBuffer<float>>& audioLookup;
    std::unordered_map<int, std::vector<float>>& cvParamLookup;
    std::vector<int> audioInputIndexes;

    const int audioOutputIndex;
    const int attackIndex, decayIndex, sustainIndex, releaseIndex;
    const int triggerInput;

    float noteOnState = 0;

    int numChannels, samplesPerBlockExpected;
    double sampleRate;

    bool readyToPlay = false;

    juce::ADSR adsr;
    juce::ADSR::Parameters currentParameters;

};
