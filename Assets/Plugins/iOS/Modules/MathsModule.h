/*
  ==============================================================================

    MathsModule.h
    Created: 20 Jun 2021 10:45:35pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include <unordered_map>
#include <functional>
#include "../Typedefs.h"
#include "Module.h"

class MathsModule : public Module
{
public:    

    // used for all other modules
    MathsModule(std::unordered_map<int, std::vector<float>>& lookup, std::vector<int> leftIn, std::vector<int> rightIn, int output, MathsModuleType t, const std::function<float(float, float)>& func, int id, float init)
        : m_CvParamLookup(lookup), m_LeftInputs(leftIn), m_RightInputs(rightIn), m_OutputIndex(output), m_MathsType(t), m_CurrentFunction(func), m_InitialValue(init)
    {
        m_ModuleId = id;
    }

    // used only for Map module
    MathsModule(std::unordered_map<int, std::vector<float>>& lookup, std::vector<int> leftIn, int output, float inMin, float inMax, float outMin, float outMax, MathsModuleType t, int id)
        : m_CvParamLookup(lookup), m_LeftInputs(leftIn), m_OutputIndex(output), m_MinIn(inMin), m_MinOut(outMin), m_MaxIn(inMax), m_MaxOut(outMax), m_MathsType(t) 
    {
        m_ModuleId = id;
    }

    ~MathsModule() = default;


    void getNextAudioBlock(int numSamples, int numChannels) override
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

    void setReady(bool state) override 
    {
        m_ReadyToPlay = state;
    }

    void setInputIndex(int outputIndex, int targetIndex) override
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
                    m_MinInIndex = outputIndex;
                    break;
                case 2:
                    m_MaxInIndex = outputIndex;
                    break;
                case 3:
                    m_MaxInIndex = outputIndex;
                    break;
                case 4:
                    m_MaxOutIndex = outputIndex;
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

    void removeInputIndex(int outputIndex, int targetIndex) override
    {
        juce::ScopedLock sl(m_Cs);

        if (m_MathsType == MathsModuleType::Map)
        {
            switch (targetIndex)
            {
                case 1:
                    m_MinInIndex = -1;
                    break;
                case 2:
                    m_MaxInIndex = -1;
                    break;
                case 3:
                    m_MaxInIndex = -1;
                    break;
                case 4:
                    m_MaxOutIndex = -1;
                    break;
            }
        }
        else
        {
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
    }

    float getMapValue(float inputValue, int sample)
    {
        if (m_MinInIndex != -1 && m_CvParamLookup.find(m_MinInIndex) != m_CvParamLookup.end())
        {
            m_MinIn = m_CvParamLookup[m_MinInIndex][sample];
        }

        if (m_MaxInIndex != -1 && m_CvParamLookup.find(m_MaxInIndex) != m_CvParamLookup.end())
        {
            m_MaxIn = m_CvParamLookup[m_MaxInIndex][sample];
        }

        if (m_MinOutIndex != -1 && m_CvParamLookup.find(m_MinOutIndex) != m_CvParamLookup.end())
        {
            m_MinOut = m_CvParamLookup[m_MinOutIndex][sample];
        }

        if (m_MaxOutIndex != -1 && m_CvParamLookup.find(m_MaxOutIndex) != m_CvParamLookup.end())
        {
            m_MaxOut = m_CvParamLookup[m_MaxOutIndex][sample];
        }

        return juce::jmap<float>(inputValue, m_MinIn, m_MaxIn, m_MinOut, m_MaxOut);
    }

private:
    
    std::function<float(float, float)> m_CurrentFunction;
    std::unordered_map<int, std::vector<float>>& m_CvParamLookup;

    std::vector<int> m_LeftInputs, m_RightInputs;

    int m_OutputIndex;
    MathsModuleType m_MathsType;

    // used only for Map maths function
    float m_MinIn, m_MaxIn, m_MinOut, m_MaxOut;
    int m_MinInIndex = -1, m_MaxInIndex = -1, m_MinOutIndex = -1, m_MaxOutIndex = -1;

    // used for SOME things
    float m_InitialValue;    
};
