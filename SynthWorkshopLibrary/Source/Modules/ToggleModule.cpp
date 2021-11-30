/*
  ==============================================================================

    ToggleModule.cpp
    Created: 24 Nov 2021 6:10:19pm
    Author:  ryand

  ==============================================================================
*/

#include "ToggleModule.h"
#include "BangModule.h"

using namespace SynthWorkshop::Modules;

ToggleModule::ToggleModule(int id) : Triggerable(id)
{
}

void ToggleModule::triggerCallback(Triggerable* sender, bool state)
{
    if (auto b = dynamic_cast<BangModule*>(sender))
    {
        auto reverseState = !m_State.load();
        m_State.store(reverseState);
    }
    else
    {
        m_State.store(state);
    }

    for (auto& t : m_Targets)
    {
        if (t.second == nullptr) continue;
        
        if (auto bang = dynamic_cast<BangModule*>(t.second))
        {
            bang->triggerCallback(this, true);
        }
        else
        {
            t.second->triggerCallback(this, m_State.load());
        }
    }
}

void ToggleModule::addTriggerTarget(Triggerable* target)
{
    juce::ScopedLock sl(m_Cs);
    if (target)
    {
        m_Targets.emplace(target->getModuleId(), target);
    }
}

void ToggleModule::removeTriggerTarget(int targetId)
{
    juce::ScopedLock sl(m_Cs);
    auto it = m_Targets.find(targetId);
    if (it != m_Targets.end())
    {
        m_Targets.erase(it);
    }
}
