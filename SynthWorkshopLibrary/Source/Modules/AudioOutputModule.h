/*
  ==============================================================================

    AudioOutputModule.h
    Created: 12 Jun 2021 9:05:28pm
    Author:  ryand

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <unordered_map>

class AudioOutputModule : juce::AudioSource
{
public:

    AudioOutputModule(std::unordered_map<int, juce::AudioBuffer<float>>& lookup, std::vector<int> left, std::vector<int> right, int nc) 
        : audioLookup(lookup), numChannels(nc), readyToPlay(false) {
        leftInputIndexes.swap(left);
        rightInputIndexes.swap(right);
    }

    ~AudioOutputModule() override {}

    void prepareToPlay(int spbe, double sr) override {
        sampleRate = sr;
        samplesPerBlockExpected = spbe;
    }

    void releaseResources() override {

    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override {        
        if (!readyToPlay) return;
        auto write = bufferToFill.buffer->getArrayOfWritePointers();        
        for (auto channel = 0; channel < numChannels; channel++) {
            for (int sample = 0; sample < bufferToFill.numSamples; sample++) {
                switch (channel) {
                    case 0:
                        for (auto i : leftInputIndexes) {
                            write[channel][sample] += audioLookup[i].getSample(channel, sample);                        
                        }
                        break;
                    case 1:
                        for (auto i : rightInputIndexes) {
                            write[channel][sample] += audioLookup[i].getSample(channel, sample);
                        }
                        break;
                }
            }
        }        
    }

    void setReady(bool state) {
        readyToPlay = state;
    }

private:
    
    std::unordered_map<int, juce::AudioBuffer<float>>& audioLookup;
    std::vector<int> leftInputIndexes, rightInputIndexes;
    int samplesPerBlockExpected, numChannels;
    double sampleRate;
    bool readyToPlay;

};