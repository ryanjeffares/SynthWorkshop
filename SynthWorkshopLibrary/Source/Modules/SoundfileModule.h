/*
  ==============================================================================

    SoundfileModule.h
    Created: 2 Dec 2021 11:09:13pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include "Module.h"

namespace SynthWorkshop
{
    namespace Modules
    {
        class SoundfileModule : public ProcessorModule, public Triggerable
        {
        public:

            enum class TransportState
            {
                Starting,
                Stopping
            };

            SoundfileModule(int id, int outIdx, AudioMap& audioMap, CVMap& cvMap);

            void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
            void getNextAudioBlock(int numSamples, int numChannels) override;
            void releaseResources() override;
            void setInputIndex(int outputIndex, int targetIndex) override;
            void removeInputIndex(int outputIndex, int targetIndex) override;

            void triggerCallback(Triggerable* sender, bool) override;

            void setSample(const juce::String& filePath);

        private:

            void changeState(TransportState state);

            int m_SoundOutIndex;
            std::vector<int> m_LoopInputIndexes;
            int m_LoopState = 0;

            juce::AudioFormatManager m_FormatManager;
            std::unique_ptr<juce::AudioFormatReaderSource> m_Reader;
            juce::AudioTransportSource m_TransportSource;
        };
    }
}