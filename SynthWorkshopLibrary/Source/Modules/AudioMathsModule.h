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

    AudioMathsModule(std::unordered_map<int, std::vector<float>>& cvLookup, std::unordered_map<int, juce::AudioBuffer<float>>& audioLu, std::vector<int> leftIn, std::vector<int> rightIns, int output, AudioCV acv, const std::function<float(float, float)>& func, int id)
        : m_CvParamLookup(cvLookup), m_AudioLookup(audioLu), m_LeftInputs(leftIn), m_RightInputs(rightIns), m_OutputIndex(output), m_AudioCvIncoming(acv), m_CurrentFunction(func) 
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
                // checks are performed in C# so that the module will always have the necessary connections                
                float affectingVal = 0.f;
                for (auto r : m_RightInputs)
                {
                    affectingVal += m_AudioCvIncoming == AudioCV::Audio ? m_AudioLookup[r].getSample(channel, sample) : m_CvParamLookup[r][sample];
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

private:

    std::unordered_map<int, std::vector<float>>& m_CvParamLookup;
    std::unordered_map<int, juce::AudioBuffer<float>>& m_AudioLookup;

    std::function<float(float, float)> m_CurrentFunction;

    std::vector<int> m_LeftInputs;
    std::vector<int> m_RightInputs;
    
    int m_OutputIndex;
    AudioCV m_AudioCvIncoming;

    double m_SampleRate;

};
