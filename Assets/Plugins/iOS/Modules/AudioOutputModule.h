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

#include "Module.h"

class AudioOutputModule : public Module
{
public:

    AudioOutputModule(std::unordered_map<int, juce::AudioBuffer<float>>& lookup, std::vector<int> left, std::vector<int> right, int id) 
        : m_AudioLookup(lookup), m_ReadyToPlay(false) 
    {
        m_ModuleId = id;
        m_LeftInputIndexes = left;
        m_RightInputIndexes = right;
    }

    void prepareToPlay(int spbe, double sr) override 
    {
        m_SampleRate = sr;
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override 
    {        
        if (!m_ReadyToPlay) return;

        auto write = bufferToFill.buffer->getArrayOfWritePointers();        

        for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); channel++) 
        {
            for (int sample = 0; sample < bufferToFill.numSamples; sample++) 
            {
                switch (channel) 
                {
                    case 0:
                        for (auto i : m_LeftInputIndexes) 
                        {
                            write[channel][sample] += m_AudioLookup[i].getSample(channel, sample);                        
                        }
                        break;
                    case 1:
                        for (auto i : m_RightInputIndexes) 
                        {
                            write[channel][sample] += m_AudioLookup[i].getSample(channel, sample);
                        }
                        break;
                }
            }
        }        
    }

    void setReady(bool state)
    {
        m_ReadyToPlay = state;
    }

    void setInputIndex(int outputIndex, int inputIndex) override
    {
        juce::ScopedLock sl(m_Cs);

        auto& vec = inputIndex ? m_RightInputIndexes : m_LeftInputIndexes;
        vec.push_back(outputIndex);
    }

    void removeInputIndex(int outputIndex, int inputIndex) override
    {
        juce::ScopedLock sl(m_Cs);

        auto& vec = inputIndex ? m_RightInputIndexes : m_LeftInputIndexes;
        for (auto it = vec.begin(); it != vec.end();)
        {
            if (*it == outputIndex)
            {
                it = vec.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

private:
    
    std::unordered_map<int, juce::AudioBuffer<float>>& m_AudioLookup;
    std::vector<int> m_LeftInputIndexes, m_RightInputIndexes;

    double m_SampleRate;
    bool m_ReadyToPlay;

};
