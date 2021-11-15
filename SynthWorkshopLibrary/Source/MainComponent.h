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
#include "Modules/Module.h"

using namespace nlohmann;

class MainComponent  : public juce::AudioAppComponent 
{
public:

    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sr) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    bool createModulesFromJson(const char* jsonText);

    void stopAudio();
    void resumeAudio();

    void setCvParam(int index, float value);
    float getCvParam(int index);
    void createCvBufferWithKey(int key);
    void createCvBufferWithKey(int key, float value);
    bool setModuleInputIndex(bool audioModule, bool add, int moduleId, int outputIndex, int targetIndex);    
    
    void setAudioMathsIncomingSignal(int moduleId, int type);

    void setMasterVolume(float);

    const char* helloWorld() 
    {
        return "Hello World from MainComponent";
    }

    using CVMap = std::unordered_map<int, std::vector<float>>;
    using AudioMap = std::unordered_map<int, juce::AudioBuffer<float>>;
    using FunctionMap = std::unordered_map<MathsModuleType, std::function<float(float, float)>>;

    bool createAudioMathsModule(const char* json);
    bool createMathsModule(const char* json);
    bool createAudioOutputModule(const char* json);
    bool createOscillatorModule(const char* json);
    bool createAdsrModule(const char* json);
    bool createNumberModule(const char* json);
    bool createFilterModule(const char* json);

    void destroyModule(bool audio, int moduleId);

    void clearModules();

private:

    Module* m_LastCreatedProcessorModule = nullptr;
    Module* m_LastCreatedOutputModule = nullptr;
    int m_ModuleIdToDestroy;

    void checkForUpdates();

    void createAudioMathsModuleFromJson(const json& values);
    void createMathsModuleFromJson(const json& values);
    void createAudioOutputModuleFromJson(const json& values);
    void createOscillatorModuleFromJson(const json& values);
    void createAdsrModuleFromJson(const json& values);

    std::vector<std::unique_ptr<Module>> m_AudioOutputModules;
    std::vector<std::unique_ptr<Module>> m_ProcessorModules;

    CVMap m_CvParamLookup;
    AudioMap m_AudioLookup;
    FunctionMap m_MathsFunctionLookup;

    float m_MasterVolume;
    
    std::atomic<bool> m_ModulesCreated;
    std::atomic<bool> m_ShouldStopAudio;
    std::atomic<bool> m_ModuleCreationCanProceed;
    std::atomic<bool> m_FirstRun;
    std::atomic<bool> m_NewProcessorModuleCreated, m_NewOutputModuleCreated;
    std::atomic<bool> m_ShouldDestroyProcessorModule, m_ShouldDestroyOutputModule;
    std::atomic<bool> m_ShouldClearModules;
    std::condition_variable m_WaitCondition;
    std::mutex m_Mutex;
    
    bool m_UnityThreadNotified = false;
    
    double m_SampleRate;
    int m_SamplesPerBlockExpected;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
