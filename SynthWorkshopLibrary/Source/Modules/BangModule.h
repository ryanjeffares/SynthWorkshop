/*
  ==============================================================================

    BangModule.h
    Created: 24 Nov 2021 9:46:51pm
    Author:  ryand

  ==============================================================================
*/


#pragma once

#include "Module.h"

namespace SynthWorkshop
{
    namespace Modules
    {
        class BangModule : public Triggerable
        {
        public:
            BangModule(int id);

            void triggerCallback(bool state) override;
            void setTriggerTarget(bool add, Triggerable* target) override;
        };
    }
}