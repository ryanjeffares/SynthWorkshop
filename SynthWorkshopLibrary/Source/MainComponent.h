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

    void setCvParam(int index, float value);
    float getCvParam(int index);
    
    void setMasterVolume(float);

    const char* helloWorld() 
    {
        return "Hello World from MainComponent";
    }

private:

    void createAudioMathsModule(const json& values);
    void createMathsModule(const json& values);
    void createAudioOutputModule(const json& values);
    void createOscillatorModule(const json& values);
    void createAdsrModule(const json& values);

    std::vector<std::unique_ptr<AudioOutputModule>> audioOutputModules;
    std::vector<std::unique_ptr<Module>> processorModules;

    std::unordered_map<int, std::vector<float>> cvParamLookup;
    std::unordered_map<int, juce::AudioBuffer<float>> audioLookup;

    std::unordered_map<MathsModuleType, std::function<float(float, float)>> mathsFunctionLookup;

    float masterVolume;
    
    std::atomic<bool> modulesCreated;
    std::atomic<bool> shouldStop;
    std::atomic<bool> moduleCreationCanProceed;
    std::atomic<bool> firstRun;
    std::condition_variable cv;
    std::mutex mtx;
    
    bool threadNotified = false;
    
    double sampleRate;
    int samplesPerBlockExpected;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
