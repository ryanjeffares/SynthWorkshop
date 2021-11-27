/*
  ==============================================================================

    AudioOutputModule.h
    Created: 12 Jun 2021 9:05:28pm
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
        class AudioOutputModule : public OutputModule
        {
        public:

            AudioOutputModule(AudioMap& lookup, std::vector<int> left, std::vector<int> right, int id);

            void prepareToPlay(int spbe, double sr) override;
            void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
            void setInputIndex(int outputIndex, int inputIndex) override;
            void removeInputIndex(int outputIndex, int inputIndex) override;

        private:

            std::vector<int> m_LeftInputIndexes, m_RightInputIndexes;

            double m_SampleRate;
        };
    }
}