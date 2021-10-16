/*
  ==============================================================================

    Exports.h
    Created: 12 Jun 2021 7:00:55pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include "MainComponent.h"

#ifdef SW_WINDOWS
#define EXPORT _declspec(dllexport)
#endif
#ifdef SW_APPLE
#define EXPORT __attribute__((visibility("default")))
#endif

extern "C" 
{

    EXPORT const char* helloWorld() 
    {
        return "Hello World!";
    }

    EXPORT void* initialise() 
    {
        juce::initialiseJuce_GUI();
        auto ptr = new MainComponent();
        return (void*)ptr;
    }

    EXPORT void shutdown(void* mc) 
    {
        delete (MainComponent*)mc;
        juce::shutdownJuce_GUI();
    }

    EXPORT const char* helloWorldFromMain(void* mc) 
    {
        return ((MainComponent*)mc)->helloWorld();
    }

    EXPORT void stopAudio(void* mc) 
    {
        ((MainComponent*)mc)->stopAudio();
    }

    EXPORT const char* createModulesFromJson(void* mc, const char* jsonText) 
    {
        return ((MainComponent*)mc)->createModulesFromJson(jsonText);
    }

    EXPORT void setCvParam(void* mc, int idx, float val) 
    {
        ((MainComponent*)mc)->setCvParam(idx, val);
    }

    EXPORT float getCvParam(void* mc, int idx)
    {
        return ((MainComponent*)mc)->getCvParam(idx);
    }

    EXPORT void setMasterVolume(void* mc, float value) 
    {
        ((MainComponent*)mc)->setMasterVolume(value);
    }
}
