/*
  ==============================================================================

    Module.h
    Created: 15 Oct 2021 10:47:59pm
    Author:  Ryan Jeffares

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#include "../Utils/Enums.h"

namespace SynthWorkshop
{
    namespace Modules
    {
        class Module
        {
        public:

            Module() = default;
            virtual ~Module() = default;

            using AudioMap = std::unordered_map<int, juce::AudioBuffer<float>>;
            using CVMap = std::unordered_map<int, std::vector<float>>;

            virtual void releaseResources() {}

            virtual void setReady(bool state)
            {
                m_ReadyToPlay = state;
            }

            inline int getModuleId() const { return m_ModuleId; }

        protected:

            bool m_ReadyToPlay = false;
            int m_ModuleId;

            juce::CriticalSection m_Cs;
        };

        class ProcessorModule : virtual public Module
        {
        public:

            ProcessorModule(int id, AudioMap& audioLookup, CVMap& cvMap)
                : m_AudioLookup(audioLookup), m_CvParamLookup(cvMap)
            {
                m_ModuleId = id;
            }

            ProcessorModule() = delete;

            virtual void prepareToPlay(int samplesPerBlockExpected, double sampleRate) = 0;
            virtual void getNextAudioBlock(int numSamples, int numChannels) = 0;

            virtual void setInputIndex(int outputIndex, int targetIndex) = 0;
            virtual void removeInputIndex(int outputIndex, int targetIndex) = 0;

            // virtual Triggerable* asTriggerable() {}

        protected:

            AudioMap& m_AudioLookup;
            CVMap& m_CvParamLookup;
        };

        class OutputModule : virtual public Module
        {
        public:

            OutputModule(int id, AudioMap& audioLookup)
                : m_AudioLookup(audioLookup)
            {
                m_ModuleId = id;
            }

            OutputModule() = delete;

            virtual void prepareToPlay(int samplesPerBlockExpected, double sampleRate) = 0;
            virtual void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) = 0;

            virtual void setInputIndex(int outputIndex, int targetIndex) = 0;
            virtual void removeInputIndex(int outputIndex, int targetIndex) = 0;

        protected:

            AudioMap& m_AudioLookup;
        };

        class Triggerable : virtual public Module
        {
        public:

            Triggerable(int id) 
            {
                m_ModuleId = id;
            }
            Triggerable() = delete;

            virtual void triggerCallback(bool) = 0;
            virtual void setTriggerTarget(bool add, Triggerable* target) {}

        protected:

            std::vector<Triggerable*> m_Targets;
        };
    }
}
