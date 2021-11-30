/*
  ==============================================================================

    MathsModule.cpp
    Created: 24 Nov 2021 5:56:59pm
    Author:  ryand

  ==============================================================================
*/

#include "MathsModule.h"

using namespace SynthWorkshop::Modules;
using namespace SynthWorkshop::Enums;

// used only for Map module

MathsModule::MathsModule(AudioMap& audioLookup, CVMap& cvLookup, std::vector<int> leftIn, int output, float inMin, float inMax, float outMin, float outMax, MathsModuleType t, int id)
    : ProcessorModule(id, audioLookup, cvLookup), 
    m_LeftInputs(leftIn), 
    m_OutputIndex(output), 
    m_MinIn(inMin), 
    m_MinOut(outMin), 
    m_MaxIn(inMax), 
    m_MaxOut(outMax), 
    m_MathsType(t)
{
}

// used for all other modules

MathsModule::MathsModule(AudioMap& audioLookup, CVMap& cvLookup, std::vector<int> leftIn, std::vector<int> rightIn, int output, MathsModuleType t, const std::function<float(float, float)>& func, int id, float init)
    : ProcessorModule(id, audioLookup, cvLookup), 
    m_LeftInputs(leftIn), 
    m_RightInputs(rightIn), 
    m_OutputIndex(output), 
    m_MathsType(t), 
    m_CurrentFunction(func), 
    m_InitialValue(init)
{
}

void MathsModule::getNextAudioBlock(int numSamples, int numChannels)
{
    if (m_OutputIndex == -1 || !m_ReadyToPlay) return;

    for (auto sample = 0; sample < numSamples; sample++)
    {
        float left = 0.f;

        for (auto i : m_LeftInputs)
        {
            left += m_CvParamLookup[i][sample];
        }

        if (m_MathsType == MathsModuleType::Map)
        {
            m_CvParamLookup[m_OutputIndex][sample] = getMapValue(left, sample);
        }
        else
        {
            float right = 0.f;
            if (m_RightInputs.size() > 0)
            {
                for (auto i : m_RightInputs)
                {
                    right += m_CvParamLookup[i][sample];
                }

                m_InitialValue = right;
            }
            else
            {
                right = m_InitialValue;
            }

            m_CvParamLookup[m_OutputIndex][sample] = m_CurrentFunction(left, right);
        }
    }
}

void MathsModule::setInputIndex(int outputIndex, int targetIndex)
{
    juce::ScopedLock sl(m_Cs);

    if (m_MathsType == MathsModuleType::Map)
    {
        switch (targetIndex)
        {
            case 0:
                m_LeftInputs.push_back(outputIndex);
                break;
            case 1:
                m_MinInIndexes.push_back(outputIndex);
                break;
            case 2:
                m_MaxInIndexes.push_back(outputIndex);
                break;
            case 3:
                m_MaxInIndexes.push_back(outputIndex);
                break;
            case 4:
                m_MaxOutIndexes.push_back(outputIndex);
                break;
        }
    }
    else
    {
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
}

void MathsModule::removeInputIndex(int outputIndex, int targetIndex)
{
    juce::ScopedLock sl(m_Cs);

    std::vector<int>* vec = nullptr;
    if (m_MathsType == MathsModuleType::Map)
    {
        switch (targetIndex)
        {
            case 1:
                vec = &m_MinInIndexes;
                break;
            case 2:
                vec = &m_MaxInIndexes;
                break;
            case 3:
                vec = &m_MaxInIndexes;
                break;
            case 4:
                vec = &m_MaxOutIndexes;
                break;
        }
    }
    else
    {
        switch (targetIndex)
        {
            case 0:
                vec = &m_LeftInputs;
                break;
            case 1:
                vec = &m_RightInputs;
                break;
        }
    }

    if (vec == nullptr) return;

    auto it = std::find(vec->begin(), vec->end(), outputIndex);
    if (it != vec->end())
    {
        vec->erase(it);
    }
}

float MathsModule::getMapValue(float inputValue, int sample)
{
    if (!m_MinInIndexes.empty())
    {
        float minIn = 0;
        for (auto i : m_MinInIndexes)
        {
            if (m_CvParamLookup.find(i) != m_CvParamLookup.end())
            {
                minIn += m_CvParamLookup[i][sample];
            }
        }
        m_MinIn = minIn;
    }

    if (!m_MaxInIndexes.empty())
    {
        float maxIn = 0;
        for (auto i : m_MaxInIndexes)
        {
            if (m_CvParamLookup.find(i) != m_CvParamLookup.end())
            {
                maxIn += m_CvParamLookup[i][sample];
            }
        }
        m_MaxIn = maxIn;
    }

    if (!m_MinOutIndexes.empty())
    {
        float minOut = 0;
        for (auto i : m_MinOutIndexes)
        {
            if (m_CvParamLookup.find(i) != m_CvParamLookup.end())
            {
                minOut += m_CvParamLookup[i][sample];
            }
        }
        m_MinOut = minOut;
    }

    if (!m_MaxOutIndexes.empty())
    {
        float maxOut = 0;
        for (auto i : m_MaxOutIndexes)
        {
            if (m_CvParamLookup.find(i) != m_CvParamLookup.end())
            {
                maxOut += m_CvParamLookup[i][sample];
            }
        }
        m_MaxOut = maxOut;
    }

    return juce::jmap<float>(inputValue, m_MinIn, m_MaxIn, m_MinOut, m_MaxOut);
}
