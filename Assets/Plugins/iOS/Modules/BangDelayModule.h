/*
  ==============================================================================

    BangDelayModule.h
    Created: 29 Nov 2021 11:36:50pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include "Module.h"

namespace SynthWorkshop
{
    namespace Modules
    {
        class BangDelayModule : public ProcessorModule, public Triggerable
        {
        public:
            BangDelayModule(int id, float delay, AudioMap& audioMap, CVMap& cvMap);

            void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
            void getNextAudioBlock(int numSamples, int numChannels) override;

            void setInputIndex(int outputIndex, int targetIndex) override;
            void removeInputIndex(int outputIndex, int targetIndex) override;

            void triggerCallback(Triggerable* sender, bool) override;
            void addTriggerTarget(Triggerable* target) override;
            void removeTriggerTarget(int targetId) override;

            void sendDelayedTrigger();

        private:

            std::vector<int> m_DelayLengthInputIndexes;

            float m_Delay;
            int m_DelayLengthSamples = 0;
            int m_SampleCounter = 0;
            double m_SampleRate;

            Triggerable* m_LastReceivedTrigger;
            bool m_StateToSend = false;
            std::atomic_bool m_WaitToSendBang{ false };
        };
    }
}