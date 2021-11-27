/*
  ==============================================================================

    ToggleModule.h
    Created: 20 Nov 2021 8:38:10pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include "Module.h"

namespace SynthWorkshop
{
    namespace Modules
    {
        class ToggleModule : public Triggerable
        {
        public:
            ToggleModule(int id);

            void triggerCallback(bool state) override;
            void setTriggerTarget(bool add, Triggerable* target) override;
        };
    }
}
