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

    EXPORT bool createModulesFromJson(void* mc, const char* jsonText) 
    {
        return ((MainComponent*)mc)->createModulesFromJson(jsonText);
    }

    EXPORT bool createSingleModule(void* mc, int type, const char* jsonText)
    {
        auto mainComponent = (MainComponent*)mc;
        switch (type)
        {
            case 0:
                return mainComponent->createAudioMathsModule(jsonText);
            case 1:
                return mainComponent->createMathsModule(jsonText);
            case 2:
                return mainComponent->createAudioOutputModule(jsonText);
            case 3:
                return mainComponent->createOscillatorModule(jsonText);
            case 4:
                return mainComponent->createAdsrModule(jsonText);
            default:
                return false;
        }
    }

    EXPORT void setCvParam(void* mc, int idx, float val) 
    {
        ((MainComponent*)mc)->setCvParam(idx, val);
    }

    EXPORT float getCvParam(void* mc, int idx)
    {
        return ((MainComponent*)mc)->getCvParam(idx);
    }

    EXPORT void createCvBufferWithKey(void* mc, int key)
    {
        ((MainComponent*)mc)->createCvBufferWithKey(key);
    }

    EXPORT void setMasterVolume(void* mc, float value) 
    {
        ((MainComponent*)mc)->setMasterVolume(value);
    }
}
