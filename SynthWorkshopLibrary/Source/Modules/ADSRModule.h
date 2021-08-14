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

class ADSRModule
{
public:

    ADSRModule(std::unordered_map<int, juce::AudioBuffer<float>>& audioLu, std::unordered_map<int, float>& cvLookup, std::vector<int> inputIndexes, int outputIndex, int numCh,
        int a, int d, int s, int r, int t) 
        : audioLookup(audioLu), cvParamLookup(cvLookup), audioOutputIndex(outputIndex), numChannels(numCh), attackIndex(a), decayIndex(d), sustainIndex(s), releaseIndex(r), triggerInput(t) 
    {
        audioInputIndexes = inputIndexes;
    }

    void prepareToPlay(int spbe, double sr) {
        samplesPerBlockExpected = spbe;
        sampleRate = sr;
        adsr.setSampleRate(sampleRate);
    }

    void releaseResources() {

    }

    void getNextAudioBlock() {
        if (!readyToPlay || audioOutputIndex == -1) return;
        updateEnvelope();
        audioLookup[audioOutputIndex].clear();
        auto write = audioLookup[audioOutputIndex].getArrayOfWritePointers();
        for (int sample = 0; sample < audioLookup[audioOutputIndex].getNumSamples(); sample++) {
            float value = adsr.getNextSample();
            for (int channel = 0; channel < audioLookup[audioOutputIndex].getNumChannels(); channel++) {
                for (const int& i : audioInputIndexes) {
                    write[channel][sample] += (audioLookup[i].getSample(channel, sample) * value);
                }
            }
        }
    }

    void setReady(bool status) {
        readyToPlay = status;
    }

private:
    
    void updateEnvelope() {
        currentParameters.attack = cvParamLookup[attackIndex];
        currentParameters.decay = cvParamLookup[decayIndex];
        currentParameters.sustain = cvParamLookup[sustainIndex];
        currentParameters.release = cvParamLookup[releaseIndex];
        adsr.setParameters(currentParameters);
        if (triggerInput != -1 && noteOnState != cvParamLookup[triggerInput]) {
            noteOnState = cvParamLookup[triggerInput];
            if (noteOnState) { 
                adsr.noteOn(); 
            }
            else { 
                adsr.noteOff(); 
            }
        }
    }

    std::unordered_map<int, juce::AudioBuffer<float>>& audioLookup;
    std::unordered_map<int, float>& cvParamLookup;
    std::vector<int> audioInputIndexes;

    const int audioOutputIndex;
    const int attackIndex, decayIndex, sustainIndex, releaseIndex;
    const int triggerInput;

    int noteOnState = 0;

    int numChannels, samplesPerBlockExpected;
    double sampleRate;

    bool readyToPlay = false;

    juce::ADSR adsr;
    juce::ADSR::Parameters currentParameters;

};
