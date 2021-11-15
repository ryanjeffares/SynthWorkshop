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

    ADSRModule(std::unordered_map<int, juce::AudioBuffer<float>>& audioLu, std::unordered_map<int, std::vector<float>>& cvLookup, std::vector<int> inputIndexes, int outputIndex,
        int a, int d, int s, int r, int t, int id) 
        : m_AudioLookup(audioLu), m_CvParamLookup(cvLookup), m_AudioOutputIndex(outputIndex), m_AttackIndex(a), m_DecayIndex(d), m_SustainIndex(s), m_ReleaseIndex(r), m_TriggerInput(t)
    {
        m_ModuleId = id;
        m_AudioInputIndexes = inputIndexes;
    }

    void prepareToPlay(int spbe, double sr) override 
    {
        m_SampleRate = sr;
        m_Envelope.setSampleRate(m_SampleRate);
    }

    void getNextAudioBlock(int numSamples, int numChannels) override 
    {
        if (!m_ReadyToPlay || m_AudioOutputIndex == -1) return;
        
        m_AudioLookup[m_AudioOutputIndex].clear();
        auto write = m_AudioLookup[m_AudioOutputIndex].getArrayOfWritePointers();
        
        for (int sample = 0; sample < numSamples; sample++) 
        {            
            m_CurrentParameters.attack = m_CvParamLookup[m_AttackIndex][sample];
            m_CurrentParameters.decay = m_CvParamLookup[m_DecayIndex][sample];
            m_CurrentParameters.sustain = m_CvParamLookup[m_SustainIndex][sample];
            m_CurrentParameters.release = m_CvParamLookup[m_ReleaseIndex][sample];

            m_Envelope.setParameters(m_CurrentParameters);

            if (m_TriggerInput != -1 && m_NoteOnState != m_CvParamLookup[m_TriggerInput][0])
            {
                m_NoteOnState = m_CvParamLookup[m_TriggerInput][0];
                if (m_NoteOnState)
                {
                    m_Envelope.noteOn();
                }
                else
                {
                    m_Envelope.noteOff();
                }
            }

            float value = m_Envelope.getNextSample();
            
            for (int channel = 0; channel < numChannels; channel++) 
            {
                for (int i : m_AudioInputIndexes) 
                {
                    write[channel][sample] += (m_AudioLookup[i].getSample(channel, sample) * value);
                }
            }
        }
    }

    void setReady(bool state) override 
    {
        m_ReadyToPlay = state;
    }

    void setInputIndex(int outputIndex, int targetIndex)
    {

    }

    void removeInputIndex(int outputIndex, int targetIndex)
    {

    }

private:   

    std::unordered_map<int, juce::AudioBuffer<float>>& m_AudioLookup;
    std::unordered_map<int, std::vector<float>>& m_CvParamLookup;
    std::vector<int> m_AudioInputIndexes;

    int m_AudioOutputIndex;
    int m_AttackIndex, m_DecayIndex, m_SustainIndex, m_ReleaseIndex;
    int m_TriggerInput;

    float m_NoteOnState = 0;

    double m_SampleRate;

    juce::ADSR m_Envelope;
    juce::ADSR::Parameters m_CurrentParameters;

};
