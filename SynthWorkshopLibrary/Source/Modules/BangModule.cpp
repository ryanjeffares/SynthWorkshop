/*
  ==============================================================================

    BangModule.cpp
    Created: 24 Nov 2021 9:46:51pm
    Author:  ryand

  ==============================================================================
*/

#include "BangModule.h"

using namespace SynthWorkshop::Modules;

BangModule::BangModule(int id)
    : Triggerable(id)
{
}

void BangModule::triggerCallback(bool state)
{
    for (auto t : m_Targets)
    {
        if (t != nullptr)
            t->triggerCallback(state);
    }
}

void BangModule::setTriggerTarget(bool add, Triggerable* target)
{
    juce::ScopedLock sl(m_Cs);
    if (add)
    {
        m_Targets.push_back(target);
    }
    else
    {
        auto it = std::find(m_Targets.begin(), m_Targets.end(), target);
        if (it != m_Targets.end())
        {
            m_Targets.erase(it);
        }
    }
}