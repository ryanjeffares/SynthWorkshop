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

#include "Module.h"

namespace SynthWorkshop
{
    namespace Modules
    {
        class MathsModule : public ProcessorModule
        {
        public:

            // used only for Map module
            MathsModule(AudioMap& audioLookup, CVMap& cvLookup, std::vector<int> leftIn, int output, float inMin, float inMax, float outMin, float outMax, SynthWorkshop::Enums::MathsModuleType t, int id);

            // used for all other modules
            MathsModule(AudioMap& audioLookup, CVMap& cvLookup, std::vector<int> leftIn, std::vector<int> rightIn, int output, SynthWorkshop::Enums::MathsModuleType t, const std::function<float(float, float)>& func, int id, float init);

            void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {}
            void getNextAudioBlock(int numSamples, int numChannels) override;
            void setInputIndex(int outputIndex, int targetIndex) override;
            void removeInputIndex(int outputIndex, int targetIndex) override;

        private:

            float getMapValue(float inputValue, int sample);

            std::function<float(float, float)> m_CurrentFunction;

            std::vector<int> m_LeftInputs, m_RightInputs;

            int m_OutputIndex;
            SynthWorkshop::Enums::MathsModuleType m_MathsType;

            // used only for Map maths function
            float m_MinIn, m_MaxIn, m_MinOut, m_MaxOut;
            std::vector<int> m_MinInIndexes, m_MaxInIndexes, m_MinOutIndexes, m_MaxOutIndexes;

            // used for SOME things
            float m_InitialValue;
        };
    }
}
