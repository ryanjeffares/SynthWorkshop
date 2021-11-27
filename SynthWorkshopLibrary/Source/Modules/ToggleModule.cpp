/*
  ==============================================================================

    ToggleModule.cpp
    Created: 24 Nov 2021 6:10:19pm
    Author:  ryand

  ==============================================================================
*/

#include "ToggleModule.h"

using namespace SynthWorkshop::Modules;

ToggleModule::ToggleModule(int id) : Triggerable(id)
{
}

void ToggleModule::triggerCallback(bool state)
{
    for (auto t : m_Targets)
    {
        if (t != nullptr)
            t->triggerCallback(state);
    }
}

void ToggleModule::setTriggerTarget(bool add, Triggerable* target)
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
