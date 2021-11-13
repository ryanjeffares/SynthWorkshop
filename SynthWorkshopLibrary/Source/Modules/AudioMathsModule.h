/*
  ==============================================================================

    AudioMathsModule.h
    Created: 20 Jun 2021 10:57:36pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <unordered_map>
#include <functional>
#include "../Typedefs.h"
#include "Module.h"

class AudioMathsModule : public Module
{
public:

    AudioMathsModule(std::unordered_map<int, std::vector<float>>& cvLookup, std::unordered_map<int, juce::AudioBuffer<float>>& audioLu, std::vector<int> leftIn, std::vector<int> rightIns, int output, AudioCV acv, const std::function<float(float, float)>& func, int id, float init)
        : m_CvParamLookup(cvLookup), m_AudioLookup(audioLu), m_LeftInputs(leftIn), m_RightInputs(rightIns), m_OutputIndex(output), m_AudioCvIncoming(acv), m_CurrentFunction(func), m_InitialValue(init)
    {
        m_ModuleId = id;
    }

    void prepareToPlay(int spbe, double sr) override 
    {
        m_SampleRate = sr;
    }

    void getNextAudioBlock(int numSamples, int numChannels) override 
    {
        if (!m_ReadyToPlay || m_OutputIndex == -1) return;
        auto write = m_AudioLookup[m_OutputIndex].getArrayOfWritePointers();
        for (int sample = 0; sample < numSamples; sample++) 
        {
            for (int channel = 0; channel < numChannels; channel++) 
            {
                float affectingVal = 0.f;

                if (m_RightInputs.size() > 0)
                {
                    for (auto r : m_RightInputs)
                    {
                        if (m_AudioCvIncoming == AudioCV::Audio)
                        {
                            if (m_AudioLookup.find(r) != m_AudioLookup.end())
                            {
                                affectingVal += m_AudioLookup[r].getSample(channel, sample);
                            }
                        }
                        else
                        {
                            if (m_CvParamLookup.find(r) != m_CvParamLookup.end())
                            {
                                affectingVal += m_CvParamLookup[r][sample];
                            }
                        }
                    }

                    m_InitialValue = affectingVal;
                }
                else
                {
                    affectingVal = m_InitialValue;
                }
                
                float inputSampleVal = 0.f;
                for (auto l : m_LeftInputs) 
                {
                    inputSampleVal += m_AudioLookup[l].getSample(channel, sample);
                }
                
                write[channel][sample] = m_CurrentFunction(inputSampleVal, affectingVal);
            }
        }
    }

    void setReady(bool state) override 
    {
        m_ReadyToPlay = state;
    }

    void setInputIndex(int outputIndex, int targetIndex) override
    {
        juce::ScopedLock sl(m_Cs);

        switch (targetIndex)
        {
            case 0:
                m_LeftInputs.push_back(outputIndex);
                break;
            case 1:
                m_RightInputs.push_back(outputIndex);
                break;
        }
    }

    void removeInputIndex(int outputIndex, int targetIndex) override
    {
        juce::ScopedLock sl(m_Cs);

        std::vector<int>* vec = nullptr;
        switch (targetIndex)
        {
            case 0:
                vec = &m_LeftInputs;
                break;
            case 1:
                vec = &m_RightInputs;
                break;
        }

        if (vec == nullptr) return;

        for (auto it = vec->begin(); it != vec->end();)
        {
            if (*it == outputIndex)
            {
                it = vec->erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void setIncomingSignalType(AudioCV type)
    {
        juce::ScopedLock sl(m_Cs);
        m_AudioCvIncoming = type;
    }

private:

    std::unordered_map<int, std::vector<float>>& m_CvParamLookup;
    std::unordered_map<int, juce::AudioBuffer<float>>& m_AudioLookup;

    std::function<float(float, float)> m_CurrentFunction;

    std::vector<int> m_LeftInputs;
    std::vector<int> m_RightInputs;
    
    int m_OutputIndex;
    AudioCV m_AudioCvIncoming;

    double m_SampleRate;

    float m_InitialValue;
};
