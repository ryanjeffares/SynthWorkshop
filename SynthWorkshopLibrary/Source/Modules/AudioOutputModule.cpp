/*
  ==============================================================================

    AudioOutputModule.cpp
    Created: 24 Nov 2021 5:49:56pm
    Author:  ryand

  ==============================================================================
*/

#include "AudioOutputModule.h"

using namespace SynthWorkshop::Modules;

AudioOutputModule::AudioOutputModule(AudioMap& lookup, std::vector<int> left, std::vector<int> right, int id)
    : OutputModule(id, lookup), 
    m_LeftInputIndexes(left), 
    m_RightInputIndexes(right)
{
}

void AudioOutputModule::prepareToPlay(int spbe, double sr)
{
    m_SampleRate = sr;
}

void AudioOutputModule::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (!m_ReadyToPlay) return;

    auto write = bufferToFill.buffer->getArrayOfWritePointers();

    for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); channel++)
    {
        for (int sample = 0; sample < bufferToFill.numSamples; sample++)
        {
            switch (channel)
            {
                case 0:
                    for (auto i : m_LeftInputIndexes)
                    {
                        write[channel][sample] += m_AudioLookup[i].getSample(channel, sample);
                    }
                    break;
                case 1:
                    for (auto i : m_RightInputIndexes)
                    {
                        write[channel][sample] += m_AudioLookup[i].getSample(channel, sample);
                    }
                    break;
            }
        }
    }
}

void AudioOutputModule::setInputIndex(int outputIndex, int inputIndex)
{
    juce::ScopedLock sl(m_Cs);

    auto& vec = inputIndex ? m_RightInputIndexes : m_LeftInputIndexes;
    vec.push_back(outputIndex);
}

void AudioOutputModule::removeInputIndex(int outputIndex, int inputIndex)
{
    juce::ScopedLock sl(m_Cs);

    auto vec = inputIndex ? &m_RightInputIndexes : &m_LeftInputIndexes;
    auto it = std::find(vec->begin(), vec->end(), outputIndex);
    if (it != vec->end())
    {
        vec->erase(it);
    }
}
