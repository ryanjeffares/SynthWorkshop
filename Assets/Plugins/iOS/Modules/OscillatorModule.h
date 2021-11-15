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

    OscillatorModule(OscillatorType t, std::unordered_map<int, std::vector<float>>& cvLookup, std::unordered_map<int, juce::AudioBuffer<float>>& audioLu, std::vector<int> freqIdx, std::vector<int> pwIdx, int soundIdx, int cvOutIdx, int id, float freq = 100)
        : m_OscillatorType(t), m_StartingFreq(freq), m_Phase(0.f), m_CvParamLookup(cvLookup), m_FrequencyInputIndexes(freqIdx), m_PulsewidthInputIndexes(pwIdx),
          m_SoundOutIndex(soundIdx), m_CvOutIndex(cvOutIdx), m_AudioLookup(audioLu)
    {
        m_ModuleId = id;
    }

    ~OscillatorModule() = default;

    void prepareToPlay(int samplesPerBlock, double sr) override 
    {
        m_SampleRate = sr;
    }

    void getNextAudioBlock(int numSamples, int numChannels) override 
    {
        if (!m_ReadyToPlay) return;
        
        for (auto sample = 0; sample < numSamples; sample++) 
        {
            float frequency = 0;
            if (m_FrequencyInputIndexes.size() > 0)
            {
                for (auto i : m_FrequencyInputIndexes)
                {
                    frequency += m_CvParamLookup[i][sample];
                }

                m_StartingFreq = frequency;
            }
            else
            {
                frequency = m_StartingFreq;
            }

            float pulseWidth = 0.5f;
            if (m_PulsewidthInputIndexes.size() > 0)
            {
                pulseWidth = 0;
                for (auto i : m_PulsewidthInputIndexes)
                {
                    pulseWidth += m_CvParamLookup[i][sample];
                }
            }

            float value = getNextSample(frequency, pulseWidth);

            if (m_SoundOutIndex != -1) 
            {
                for (auto channel = 0; channel < numChannels; channel++) 
                {
                    m_AudioLookup[m_SoundOutIndex].setSample(channel, sample, value);
                }
            }

            if (m_CvOutIndex != -1)
            {
                m_CvParamLookup[m_CvOutIndex][sample] = value;
            }
        }
    }

    void setReady(bool state) override 
    {
        m_ReadyToPlay = state;
    }

    OscillatorType getType() const 
    {
        return m_OscillatorType;
    }

    void setType(OscillatorType newType) 
    {
        m_OscillatorType = newType;
    }

    void setInputIndex(int outputIndex, int targetIndex) override
    {        
        juce::ScopedLock sl(m_Cs);

        switch (targetIndex)
        {
            case 2:
                m_FrequencyInputIndexes.push_back(outputIndex);
                break;
            case 3:
                m_PulsewidthInputIndexes.push_back(outputIndex);
                break;
        }
    }

    void removeInputIndex(int outputIndex, int targetIndex) override
    {
        juce::ScopedLock sl(m_Cs);

        std::vector<int>* vec = nullptr;
        switch (targetIndex)
        {
            case 2:
                vec = &m_FrequencyInputIndexes;
                break;
            case 3:
                vec = &m_PulsewidthInputIndexes;
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

private:

    float getNextSample(float frequency, float pulseWidth)
    {
        switch (m_OscillatorType) 
        {
            case OscillatorType::Saw:
                m_Output = m_Phase;
                if (m_Phase >= 1.f) 
                {
                    m_Phase -= 2.f;
                }
                m_Phase += (1.f / (m_SampleRate / frequency)) * 2.f;
                return m_Output;
            case OscillatorType::Pulse:
                if (m_Phase >= 1.f) 
                {
                    m_Phase -= 1.f;
                }
                m_Phase += (1.f / (m_SampleRate / frequency));
                if (m_Phase < pulseWidth)
                {
                    m_Output = -1;
                }
                if (m_Phase > pulseWidth)
                {
                    m_Output = 1;
                }
                return m_Output;
            case OscillatorType::Sine:
                m_Output = std::sin(m_Phase * juce::MathConstants<float>::twoPi);
                if (m_Phase >= 1.f) 
                {
                    m_Phase -= 1.f;
                }
                m_Phase += (1.f / (m_SampleRate / frequency));
                return m_Output;
            case OscillatorType::Tri:
                if (m_Phase >= 1.f) 
                {
                    m_Phase -= 1.f;
                }
                m_Phase += (1.f / (m_SampleRate / frequency));
                if (m_Phase <= 0.5f) 
                {
                    m_Output = (m_Phase - 0.25f) * 4;
                }
                else 
                {
                    m_Output = ((1.f - m_Phase) - 0.25f) * 4;
                }
                return m_Output;
            default: return 0;
        }
    }

    std::unordered_map<int, juce::AudioBuffer<float>>& m_AudioLookup;
    std::unordered_map<int, std::vector<float>>& m_CvParamLookup;

    OscillatorType m_OscillatorType;    

    double m_SampleRate;

    float m_StartingFreq;
    float m_Phase, m_Output;
    int m_SoundOutIndex, m_CvOutIndex;
    std::vector<int> m_FrequencyInputIndexes, m_PulsewidthInputIndexes;

};
