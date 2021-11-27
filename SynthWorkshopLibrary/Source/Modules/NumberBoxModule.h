/*
  ==============================================================================

    NumberBoxModule.h
    Created: 14 Nov 2021 7:14:09pm
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
        class NumberBoxModule : public ProcessorModule
        {
        public:

            NumberBoxModule(CVMap& cv, AudioMap& audio, std::vector<int> inputs, int output, int id, float init);

            void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {}
            void getNextAudioBlock(int numSamples, int numChannels) override;
            void setInputIndex(int outputIndex, int targetIndex) override;
            void removeInputIndex(int outputIndex, int targetIndex) override;

            inline void setValue(float value)
            {
                m_InitialValue.store(value);
            }

        private:

            std::vector<int> m_LeftInputs;

            int m_OutputIndex;

            std::atomic<float> m_InitialValue;
        };
    }
}