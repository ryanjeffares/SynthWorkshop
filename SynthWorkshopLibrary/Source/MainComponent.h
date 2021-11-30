/*
  ==============================================================================

    MainComponent.h
    Created: 12 Jun 2021 7:00:37pm
    Author:  ryand

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include <utility>
#include <mutex>
#include <condition_variable>

#include "../Include/json.h"

#include "Modules/OscillatorModule.h"
#include "Modules/AudioOutputModule.h"
#include "Modules/MathsModule.h"
#include "Modules/AudioMathsModule.h"
#include "Modules/ADSRModule.h"
#include "Modules/NumberBoxModule.h"
#include "Modules/FilterModule.h"
#include "Modules/ToggleModule.h"
#include "Modules/BangModule.h"
#include "Modules/BangDelayModule.h"

using namespace nlohmann;

class MainComponent  : public juce::AudioAppComponent 
{
public:

    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sr) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void stopAudio();
    void resumeAudio();

    void setCvParam(int index, float value);
    float getCvParam(int index);
    bool getTriggerableState(int moduleId);

    void triggerCallback(int moduleId, bool state);

    void createCvBufferWithKey(int key);
    void createCvBufferWithKey(int key, float value);

    bool setModuleInputIndex(bool audioModule, bool add, int moduleId, int outputIndex, int targetIndex);    
    bool setTriggerTarget(bool add, int senderId, int targetId);

    void setAudioMathsIncomingSignal(int moduleId, int type);
    void setNumberBoxValue(int moduleId, float value);

    void setMasterVolume(float);

    const char* helloWorld() 
    {
        return "Hello World from MainComponent";
    }    

    bool createAudioMathsModule(const char* json);
    bool createMathsModule(const char* json);
    bool createAudioOutputModule(const char* json);
    bool createOscillatorModule(const char* json);
    bool createAdsrModule(const char* json);
    bool createNumberModule(const char* json);
    bool createFilterModule(const char* json);
    bool createToggleModule(const char* json);
    bool createBangModule(const char* json);
    bool createBangDelayModule(const char* json);

    void destroyModule(int moduleType, int moduleId);

    void clearModules();

private:

    void checkForUpdates();

    bool removeTriggerTarget(int senderId, int targetId);
    bool addTriggerTarget(int senderId, int targetId);

    SynthWorkshop::Modules::ProcessorModule* m_LastCreatedProcessorModule = nullptr;
    SynthWorkshop::Modules::OutputModule* m_LastCreatedOutputModule = nullptr;
    SynthWorkshop::Modules::Triggerable* m_LastCreatedTriggerModule = nullptr;
    int m_ModuleIdToDestroy;

    std::vector<std::unique_ptr<SynthWorkshop::Modules::ProcessorModule>> m_ProcessorModules;
    std::vector<std::unique_ptr<SynthWorkshop::Modules::OutputModule>> m_AudioOutputModules;
    std::vector<std::unique_ptr<SynthWorkshop::Modules::Triggerable>> m_TriggerModules;

    using CVMap = std::unordered_map<int, std::vector<float>>;
    using AudioMap = std::unordered_map<int, juce::AudioBuffer<float>>;
    using FunctionMap = std::unordered_map<SynthWorkshop::Enums::MathsModuleType, std::function<float(float, float)>>;

    CVMap m_CvParamLookup;
    AudioMap m_AudioLookup;
    FunctionMap m_MathsFunctionLookup;

    float m_MasterVolume;
    
    std::atomic<bool> m_ShouldStopAudio;
    std::atomic<bool> m_NewProcessorModuleCreated, m_NewOutputModuleCreated, m_NewTriggerModuleCreated;
    std::atomic<bool> m_ShouldDestroyProcessorModule, m_ShouldDestroyOutputModule, m_ShouldDestroyTriggerModule;
    std::atomic<bool> m_ShouldClearModules;

    juce::CriticalSection m_Cs;
    
    double m_SampleRate;
    int m_SamplesPerBlockExpected;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
