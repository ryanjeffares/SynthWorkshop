/*
  ==============================================================================

    BangModule.cpp
    Created: 24 Nov 2021 9:46:51pm
    Author:  ryand

  ==============================================================================
*/

#include "BangModule.h"
#include "ToggleModule.h"

using namespace SynthWorkshop::Modules;

BangModule::BangModule(int id, AudioMap& audioMap, CVMap& cvMap)
    : Triggerable(id), ProcessorModule(id, audioMap, cvMap)
{
}

void BangModule::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    m_BangTimeSamples = static_cast<int>(sampleRate * m_BangTime);
}

void BangModule::getNextAudioBlock(int numSamples, int numChannels)
{
    for (int sample = 0; sample < numSamples; sample++)
    {
        if (m_Banging.load())
        {
            m_SampleCounter++;
            if (m_SampleCounter.load() > m_BangTimeSamples)
            {
                m_Banging.store(false);
                m_State.store(false);
                m_SampleCounter = 0;
            }
        }
    }
}

void BangModule::triggerCallback(Triggerable* sender, bool state)
{    
    // toggles always trigger bangs no matter what
    if (auto toggle = dynamic_cast<ToggleModule*>(sender))
    {
        m_State.store(true);
        m_Banging.store(true);
        for (auto& t : m_Targets)
        {
            if (t.second != nullptr)
                t.second->triggerCallback(this, true);
        }
    }
    else
    {
        m_State.store(state);
        if (state)
        {
            m_Banging.store(true);
            for (auto& t : m_Targets)
            {
                if (t.second != nullptr)
                    t.second->triggerCallback(this, true);
            }
        }
    }
}

void BangModule::addTriggerTarget(Triggerable* target)
{
    juce::ScopedLock sl(m_Cs);
    if (target)
    {
        m_Targets.emplace(target->getModuleId(), target);
    }
}

void BangModule::removeTriggerTarget(int targetId)
{
    juce::ScopedLock sl(m_Cs);
    auto it = m_Targets.find(targetId);
    if (it != m_Targets.end())
    {
        m_Targets.erase(it);
    }
}