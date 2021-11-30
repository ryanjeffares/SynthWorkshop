/*
  ==============================================================================

    BangModule.h
    Created: 24 Nov 2021 9:46:51pm
    Author:  ryand

  ==============================================================================
*/


#pragma once

#include "Module.h"

namespace SynthWorkshop
{
    namespace Modules
    {
        class BangModule : public Triggerable, public ProcessorModule
        {
        public:
            BangModule(int id, AudioMap& audioMap, CVMap& cvMap);

            void triggerCallback(Triggerable* sender, bool state) override;
            void addTriggerTarget(Triggerable* target) override;
            void removeTriggerTarget(int targetId) override;

            void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
            void getNextAudioBlock(int numSamples, int numChannels) override;

            void setInputIndex(int outputIndex, int targetIndex) override {}
            void removeInputIndex(int outputIndex, int targetIndex) override {}

        private:

            const double m_BangTime = 0.2;
            int m_BangTimeSamples;
            std::atomic_int m_SampleCounter{ 0 };
            std::atomic_bool m_Banging{ false };
        };
    }
}