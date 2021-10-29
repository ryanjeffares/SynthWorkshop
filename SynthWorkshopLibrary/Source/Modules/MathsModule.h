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
    MathsModule(std::unordered_map<int, std::vector<float>>& lookup, std::vector<int> leftIn, std::vector<int> rightIn, int output, MathsModuleType t, const std::function<float(float, float)>& func, int id)
        : m_CvParamLookup(lookup), m_LeftInputs(leftIn), m_RightInputs(rightIn), m_OutputIndex(output), m_MathsType(t), m_CurrentFunction(func) 
    {
        m_ModuleId = id;
    }

    // used only for Map module
    MathsModule(std::unordered_map<int, std::vector<float>>& lookup, std::vector<int> leftIn, std::vector<int> rightIn, int output, int inMin, int inMax, int outMin, int outMax, MathsModuleType t, int id)
        : m_CvParamLookup(lookup), m_LeftInputs(leftIn), m_RightInputs(rightIn), m_OutputIndex(output), m_MinIn(inMin), m_MinOut(outMin), m_MaxIn(inMax), m_MaxOut(outMax), m_MathsType(t) 
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
            float right = 0.f;

            for (auto i : m_LeftInputs)
            {
                left += m_CvParamLookup[i][sample];
            }

            for (auto i : m_RightInputs)
            {
                right += m_CvParamLookup[i][sample];
            }

            if (m_MathsType == MathsModuleType::Map)
            {
                m_CvParamLookup[m_OutputIndex][sample] = juce::jmap<float>(
                    left, m_CvParamLookup[m_MinIn][sample], m_CvParamLookup[m_MaxIn][sample], m_CvParamLookup[m_MinOut][sample], m_CvParamLookup[m_MaxOut][sample]
                );
            }
            else
            {
                m_CvParamLookup[m_OutputIndex][sample] = m_CurrentFunction(left, right);
            }
        }        
    }

    void setReady(bool state) override 
    {
        m_ReadyToPlay = state;
    }

private:
    
    std::function<float(float, float)> m_CurrentFunction;
    std::unordered_map<int, std::vector<float>>& m_CvParamLookup;

    std::vector<int> m_LeftInputs, m_RightInputs;

    int m_OutputIndex;
    MathsModuleType m_MathsType;

    // used only for Map maths function
    int m_MinIn, m_MaxIn, m_MinOut, m_MaxOut;

};
