/*
  ==============================================================================

    NumberBoxModule.cpp
    Created: 24 Nov 2021 6:00:24pm
    Author:  ryand

  ==============================================================================
*/

#include "NumberBoxModule.h"

using namespace SynthWorkshop::Modules;

NumberBoxModule::NumberBoxModule(CVMap& cv, AudioMap& audio, std::vector<int> inputs, int output, int id, float init)
    : ProcessorModule(id, audio, cv), 
    m_LeftInputs(inputs), 
    m_OutputIndex(output), 
    m_InitialValue(init)
{
}

void NumberBoxModule::getNextAudioBlock(int numSamples, int numChannels)
{
    if (!m_ReadyToPlay || m_OutputIndex == -1) return;

    for (int sample = 0; sample < numSamples; sample++)
    {
        if (!m_LeftInputs.empty())
        {
            float value = 0;
            for (auto l : m_LeftInputs)
            {
                value += m_CvParamLookup[l][sample];
            }

            m_InitialValue.store(value);
        }

        for (int i = 0; i < m_CvParamLookup[m_OutputIndex].size(); i++)
        {
            m_CvParamLookup[m_OutputIndex][i] = m_InitialValue.load();
        }
    }
}

void NumberBoxModule::setInputIndex(int outputIndex, int targetIndex)
{
    juce::ScopedLock sl(m_Cs);

    if (targetIndex == 0)
    {
        m_LeftInputs.push_back(outputIndex);
    }
}

void NumberBoxModule::removeInputIndex(int outputIndex, int targetIndex)
{
    juce::ScopedLock sl(m_Cs);

    auto it = std::find(m_LeftInputs.begin(), m_LeftInputs.end(), outputIndex);
    if (it != m_LeftInputs.end())
    {
        m_LeftInputs.erase(it);
    }
}
