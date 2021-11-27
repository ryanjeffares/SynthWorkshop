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

#include "Module.h"

namespace SynthWorkshop
{
    namespace Modules
    {
        class AudioMathsModule : public ProcessorModule
        {
        public:

            AudioMathsModule(CVMap& cvLookup, AudioMap& audioLu, std::vector<int> leftIn, std::vector<int> rightIns, int output, SynthWorkshop::Enums::AudioCV acv, const std::function<float(float, float)>& func, int id, float init);

            void prepareToPlay(int spbe, double sr) override;
            void getNextAudioBlock(int numSamples, int numChannels) override;
            void setInputIndex(int outputIndex, int targetIndex) override;
            void removeInputIndex(int outputIndex, int targetIndex) override;

            void setIncomingSignalType(SynthWorkshop::Enums::AudioCV type);

        private:

            std::function<float(float, float)> m_CurrentFunction;

            std::vector<int> m_LeftInputs;
            std::vector<int> m_RightInputs;
    
            int m_OutputIndex;
            SynthWorkshop::Enums::AudioCV m_AudioCvIncoming;

            double m_SampleRate;

            float m_InitialValue;
        };
    }
}
