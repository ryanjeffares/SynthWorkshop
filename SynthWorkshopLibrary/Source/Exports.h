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
        ((MainComponent*)mc)->clearModules();
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

    EXPORT void destroyModule(void* mc, bool audio, int moduleId)
    {
        auto ptr = (MainComponent*)mc;

        // this function is being called on scene exit because OnDestroy() is called for each game object
        // it causes crashes i SUSPECT because MainComponent is destroyed before its called
        if (ptr != nullptr)
        {
            ptr->destroyModule(audio, moduleId);
        }
    }

    EXPORT bool setModuleInputIndex(void* mc, bool audioModule, bool add, int moduleId, int outputIndex, int targetIndex)
    {
        if (mc == nullptr)
        {
            return false;
        }

        return ((MainComponent*)mc)->setModuleInputIndex(audioModule, add, moduleId, outputIndex, targetIndex);
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

    EXPORT void setAudioMathsIncomingSignal(void* mc, int id, int type)
    {
        ((MainComponent*)mc)->setAudioMathsIncomingSignal(id, type);
    }

    EXPORT void setMasterVolume(void* mc, float value) 
    {
        ((MainComponent*)mc)->setMasterVolume(value);
    }
}
