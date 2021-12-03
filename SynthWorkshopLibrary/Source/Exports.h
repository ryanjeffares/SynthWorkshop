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
        auto main = (MainComponent*)mc;
        if (main == nullptr) return;
        delete main;
        juce::shutdownJuce_GUI();
    }

    EXPORT const char* helloWorldFromMain(void* mc) 
    {
        if (mc == nullptr) return "";
        return ((MainComponent*)mc)->helloWorld();
    }

    EXPORT void stopAudio(void* mc) 
    {
        if (mc == nullptr) return;
        ((MainComponent*)mc)->stopAudio();
    }

    EXPORT void resumeAudio(void* mc)
    {
        if (mc == nullptr) return;
        ((MainComponent*)mc)->resumeAudio();
    }

    EXPORT bool createSingleModule(void* mc, int type, const char* jsonText)
    {
        auto mainComponent = (MainComponent*)mc;
        if (mainComponent == nullptr) return false;
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
            case 5:
                return mainComponent->createNumberModule(jsonText);
            case 6:
                return mainComponent->createFilterModule(jsonText);
            case 7:
                return mainComponent->createToggleModule(jsonText);
            case 8:
                return mainComponent->createBangModule(jsonText);
            case 9:
                return mainComponent->createBangDelayModule(jsonText);
            case 10:
                return mainComponent->createSoundfileModule(jsonText);
            default:
                return false;
        }
    }

    EXPORT void destroyModule(void* mc, int moduleType, int moduleId)
    {
        auto ptr = (MainComponent*)mc;

        if (ptr != nullptr)
        {
            ptr->destroyModule(moduleType, moduleId);
        }
    }

    EXPORT void clearAllModules(void* mc)
    {
        ((MainComponent*)mc)->clearModules();
    }

    EXPORT bool setModuleInputIndex(void* mc, bool audioModule, bool add, int moduleId, int outputIndex, int targetIndex)
    {
        if (mc == nullptr) return false;
        return ((MainComponent*)mc)->setModuleInputIndex(audioModule, add, moduleId, outputIndex, targetIndex);
    }

    EXPORT bool setTriggerTarget(void* mc, bool add, int senderId, int targetId)
    {
        if (mc == nullptr) return false;
        return ((MainComponent*)mc)->setTriggerTarget(add, senderId, targetId);
    }

    EXPORT void setCvParam(void* mc, int idx, float val) 
    {
        if (mc == nullptr) return;
        ((MainComponent*)mc)->setCvParam(idx, val);
    }

    EXPORT float getCvParam(void* mc, int idx)
    {
        if (mc == nullptr) return 0;
        return ((MainComponent*)mc)->getCvParam(idx);
    }

    EXPORT bool getTriggerableState(void* mc, int id)
    {
        return ((MainComponent*)mc)->getTriggerableState(id);
    }

    EXPORT void triggerCallback(void* mc, int moduleId, bool state)
    {
        if (mc == nullptr) return;
        ((MainComponent*)mc)->triggerCallback(moduleId, state);
    }    

    EXPORT void createCvBufferWithKey(void* mc, int key)
    {
        if (mc == nullptr) return;
        ((MainComponent*)mc)->createCvBufferWithKey(key);
    }

    EXPORT void createCvBufferWithKeyAndValue(void* mc, int key, float value)
    {
        if (mc == nullptr) return;
        ((MainComponent*)mc)->createCvBufferWithKey(key, value);
    }

    EXPORT void setAudioMathsIncomingSignal(void* mc, int id, int type)
    {
        if (mc == nullptr) return;
        ((MainComponent*)mc)->setAudioMathsIncomingSignal(id, type);
    }

    EXPORT void setNumberBoxValue(void* mc, int id, float value)
    {
        if (mc == nullptr) return;
        ((MainComponent*)mc)->setNumberBoxValue(id, value);
    }

    EXPORT bool setSoundfileModuleSample(void* mc, int moduleId, const char* filePath)
    {
        return ((MainComponent*)mc)->setSoundfileModuleSample(moduleId, filePath);
    }

    EXPORT void setMasterVolume(void* mc, float value) 
    {
        if (mc == nullptr) return;
        ((MainComponent*)mc)->setMasterVolume(value);
    }
}
