/*
  ==============================================================================

    SoundfileModule.cpp
    Created: 2 Dec 2021 11:09:13pm
    Author:  ryand

  ==============================================================================
*/

#include "ToggleModule.h"
#include "BangModule.h"
#include "SoundfileModule.h"

using namespace SynthWorkshop::Modules;

SoundfileModule::SoundfileModule(int id, int outIdx, AudioMap& audioMap, CVMap& cvMap)
    : ProcessorModule(id, audioMap, cvMap), Triggerable(id), m_SoundOutIndex(outIdx)
{
    m_FormatManager.registerBasicFormats();
}

void SoundfileModule::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    m_TransportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void SoundfileModule::getNextAudioBlock(int numSamples, int numChannels)
{
    if (!m_ReadyToPlay) return;

    if (m_Reader.get() == nullptr)
    {
        m_AudioLookup[m_SoundOutIndex].clear();
    }

    int shouldLoop = 0;
    if (!m_LoopInputIndexes.empty())
    {
        float loop = 0;
        for (auto i : m_LoopInputIndexes)
        {
            loop += m_CvParamLookup[i][0];
        }

        shouldLoop = static_cast<int>(loop);
    }

    if (m_LoopState != shouldLoop)
    {
        m_LoopState = shouldLoop;
        m_TransportSource.setLooping(m_LoopState > 0);
    }

    juce::AudioSourceChannelInfo channelInfo(m_AudioLookup[m_SoundOutIndex]);
    m_TransportSource.getNextAudioBlock(channelInfo);    
}

void SoundfileModule::releaseResources()
{
    m_TransportSource.releaseResources();
}

void SoundfileModule::setInputIndex(int outputIndex, int targetIndex)
{
    if (targetIndex == 1)
    {
        if (m_CvParamLookup.find(outputIndex) != m_CvParamLookup.end())
        {
            m_LoopInputIndexes.push_back(outputIndex);
        }
    }
}

void SoundfileModule::removeInputIndex(int outputIndex, int targetIndex)
{
    if (targetIndex == 1)
    {
        juce::ScopedLock sl(m_Cs);
        auto it = std::find(m_LoopInputIndexes.begin(), m_LoopInputIndexes.end(), outputIndex);
        m_LoopInputIndexes.erase(it);
    }
}

void SoundfileModule::triggerCallback(Triggerable* sender, bool state)
{
    if (auto bang = dynamic_cast<BangModule*>(sender))
    {
        changeState(TransportState::Starting);
    }
    else// if (auto toggle = dynamic_cast<ToggleModule*>(sender))
    {
        changeState(state ? TransportState::Starting : TransportState::Stopping);
    }
}

void SoundfileModule::setSample(const juce::String& filePath)
{
    juce::File file(filePath);
    if (file.existsAsFile())
    {
        auto reader = m_FormatManager.createReaderFor(file);
        if (reader != nullptr)
        {
            auto source = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
            m_TransportSource.setSource(source.get(), 0, nullptr, reader->sampleRate);
            m_Reader.reset(source.release());
        }
    }
}

void SoundfileModule::changeState(TransportState state)
{
    switch (state)
    {
        case TransportState::Starting:
            m_TransportSource.setPosition(0.0);
            m_TransportSource.start();
            break;
        case TransportState::Stopping:
            m_TransportSource.stop();
            break;
    }
}