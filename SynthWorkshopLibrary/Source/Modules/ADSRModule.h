/*
  ==============================================================================

    ADSRModule.h
    Created: 23 Jun 2021 9:28:26pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <vector>
#include <unordered_map>

#include "Module.h"

namespace SynthWorkshop
{
    namespace Modules
    {
        class ADSRModule : public ProcessorModule, public Triggerable
        {
        public:

            ADSRModule(AudioMap& audioLu, CVMap& cvLookup, std::vector<int> inputIndexes, int outputIndex, float attack, float decay, float sustain, float release, int id);

            void prepareToPlay(int spbe, double sr) override;
            void getNextAudioBlock(int numSamples, int numChannels) override;
            void setInputIndex(int outputIndex, int targetIndex) override;
            void removeInputIndex(int outputIndex, int targetIndex) override;

            // Triggerable* asTriggerable() override;

            void triggerCallback(Triggerable* sender, bool state) override;

        private:

            void updateEnvelope(int sample);

            std::vector<int> m_AudioInputIndexes;

            int m_AudioOutputIndex;

            float m_Attack, m_Decay, m_Sustain, m_Release;
            std::vector<int> m_AttackIndexes, m_DecayIndexes, m_SustainIndexes, m_ReleaseIndexes;
            std::vector<int> m_TriggerInputs;

            double m_SampleRate;

            juce::ADSR m_Envelope;
            std::atomic<bool> m_EnvelopeShouldNoteOn, m_EnvelopeShouldNoteOff;
        };
    }
}
