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

#include "Module.h"

namespace SynthWorkshop
{
    namespace Modules
    {
        class OscillatorModule : public ProcessorModule, public Triggerable
        {
        public:

            OscillatorModule(SynthWorkshop::Enums::OscillatorType t, CVMap& cvLookup, AudioMap& audioLu, std::vector<int> freqIdx, std::vector<int> pwIdx, int soundIdx, int cvOutIdx, int id, float freq = 100);

            void prepareToPlay(int samplesPerBlock, double sr) override;
            void getNextAudioBlock(int numSamples, int numChannels) override;
            void setInputIndex(int outputIndex, int targetIndex) override;
            void removeInputIndex(int outputIndex, int targetIndex) override;

            void triggerCallback(bool) override;

        private:

            float getNextSample(float frequency, float pulseWidth);

            SynthWorkshop::Enums::OscillatorType m_OscillatorType;

            double m_SampleRate;

            float m_StartingFreq;
            float m_Phase, m_Output;
            int m_SoundOutIndex, m_CvOutIndex;
            std::vector<int> m_FrequencyInputIndexes, m_PulsewidthInputIndexes;

            std::atomic<bool> m_ShouldResetPhase;

        };
    }
}
