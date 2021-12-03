/*
  ==============================================================================

    BangDelayModule.cpp
    Created: 29 Nov 2021 11:36:50pm
    Author:  ryand

  ==============================================================================
*/

#include "BangDelayModule.h"

using namespace SynthWorkshop::Modules;

BangDelayModule::BangDelayModule(int id, float delay, AudioMap& audioMap, CVMap& cvMap)
    : Triggerable(id), ProcessorModule(id, audioMap, cvMap), m_Delay(delay)
{
}

void BangDelayModule::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    m_SampleRate = sampleRate;
    auto length = m_SampleRate * m_Delay;
    m_DelayLengthSamples = static_cast<int>(length);
}

void BangDelayModule::getNextAudioBlock(int numSamples, int numChannels)
{
    if (!m_ReadyToPlay) return;

    for (auto sample = 0; sample < numSamples; sample++)
    {
        float delay = 0;
        if (!m_DelayLengthInputIndexes.empty())
        {
            for (auto i : m_DelayLengthInputIndexes)
            {
                delay += m_CvParamLookup[i][sample];
            }

            m_Delay = delay;
            auto length = m_SampleRate * m_Delay;
            m_DelayLengthSamples = static_cast<int>(length);
        }

        if (m_WaitToSendBang.load())
        {
            m_SampleCounter++;
            if (m_SampleCounter >= m_DelayLengthSamples)
            {
                m_SampleCounter = 0;
                m_WaitToSendBang.store(false);
                sendDelayedTrigger();
            }
        }
    }
}

void BangDelayModule::setInputIndex(int outputIndex, int targetIndex)
{
    if (targetIndex == 1)
    {
        juce::ScopedLock sl(m_Cs);
        m_DelayLengthInputIndexes.push_back(outputIndex);
    }
}

void BangDelayModule::removeInputIndex(int outputIndex, int targetIndex)
{
    if (targetIndex == 1)
    {
        juce::ScopedLock sl(m_Cs);
        auto it = std::find(m_DelayLengthInputIndexes.begin(), m_DelayLengthInputIndexes.end(), outputIndex);
        if (it != m_DelayLengthInputIndexes.end())
        {
            m_DelayLengthInputIndexes.erase(it);
        }
    }
}

void BangDelayModule::triggerCallback(Triggerable* sender, bool state)
{
    m_LastReceivedTrigger = sender;
    m_StateToSend = state;
    m_WaitToSendBang.store(true);
}

void BangDelayModule::addTriggerTarget(Triggerable* target)
{
    juce::ScopedLock sl(m_Cs);
    if (target)
    {
        m_Targets.emplace(target->getModuleId(), target);
    }
}

void BangDelayModule::removeTriggerTarget(int targetId)
{
    juce::ScopedLock sl(m_Cs);
    auto it = m_Targets.find(targetId);
    if (it != m_Targets.end())
    {
        m_Targets.erase(it);
    }
}

void BangDelayModule::sendDelayedTrigger()
{
    for (auto t : m_Targets)
    {
        if (t.second != nullptr)
        {
            // so its as if the original sender is calling it?
            t.second->triggerCallback(m_LastReceivedTrigger, m_StateToSend);
        }
    }
}
