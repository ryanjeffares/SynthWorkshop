/*
  ==============================================================================

    NumberBoxModule.h
    Created: 14 Nov 2021 7:14:09pm
    Author:  ryand

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "Module.h"

class NumberBoxModule : public Module
{
public:

    NumberBoxModule(std::unordered_map<int, std::vector<float>>& cv, std::unordered_map<int, juce::AudioBuffer<float>>& audio, std::vector<int> inputs, int output, int id, float init) 
        : m_CvParamLookup(cv), m_AudioLookup(audio), m_LeftInputs(inputs), m_OutputIndex(output), m_InitialValue(init) 
    {
        m_ModuleId = id;
    }

    void getNextAudioBlock(int numSamples, int numChannels)
    {
        if (!m_ReadyToPlay) return;

        for (int sample = 0; sample < numSamples; sample++)
        {
            float value = 0;
            if (m_LeftInputs.size() > 0)
            {
                for (auto l : m_LeftInputs)
                {
                    value += m_CvParamLookup[l][sample];
                }

                m_InitialValue = value;
            }

            for (int i = 0; i < m_CvParamLookup[m_OutputIndex].size(); i++)
            {
                m_CvParamLookup[m_OutputIndex][i] = m_InitialValue;
            }
        }
    }

    void setReady(bool state)
    {
        m_ReadyToPlay = state;
    }

    void setInputIndex(int outputIndex, int targetIndex)
    {
        juce::ScopedLock sl(m_Cs);

        if (targetIndex == 0)
        {
            m_LeftInputs.push_back(outputIndex);
        }
    }

    void removeInputIndex(int outputIndex, int targetIndex)
    {
        juce::ScopedLock sl(m_Cs);

        for (auto it = m_LeftInputs.begin(); it != m_LeftInputs.end();)
        {
            if (*it == outputIndex)
            {
                it = m_LeftInputs.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

private:

    std::unordered_map<int, std::vector<float>>& m_CvParamLookup;
    std::unordered_map<int, juce::AudioBuffer<float>>& m_AudioLookup;

    std::vector<int> m_LeftInputs;

    int m_OutputIndex;

    float m_InitialValue;    
};