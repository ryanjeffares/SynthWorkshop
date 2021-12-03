/*
  ==============================================================================

    FilterModule.h
    Created: 14 Nov 2021 8:29:31pm
    Author:  ryand

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "Module.h"

namespace SynthWorkshop
{
    namespace Modules
    {
        class FilterModule : public ProcessorModule
        {
        public:

            using FiltType = juce::dsp::StateVariableTPTFilter<float>::Type;

            FilterModule(AudioMap& audio, CVMap& cv, std::vector<int> audioInputs, std::vector<int> freqInputs, std::vector<int> qInputs, std::vector<int> typeInputs, int output, int id);
            FilterModule(AudioMap& audio, CVMap& cv, std::vector<int> audioInputs, std::vector<int> freqInputs, std::vector<int> qInputs, std::vector<int> typeInputs, int output, int id, float cutoff, float q, int type);

            void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
            void getNextAudioBlock(int numSamples, int numChannels) override;
            void releaseResources() override;
            void setInputIndex(int outputIndex, int targetIndex) override;
            void removeInputIndex(int outputIndex, int targetIndex) override;

        private:

            juce::dsp::StateVariableTPTFilter<float> m_Filter;

            std::vector<int> m_AudioInputIndexes;

            std::vector<int> m_FreqInputIndexes, m_QInputIndexes, m_TypeInputIndexes;

            int m_OutputIndex;
            double m_SampleRate;

            FiltType m_FilterType;
            float m_Cutoff, m_Q;
        };
    }
}