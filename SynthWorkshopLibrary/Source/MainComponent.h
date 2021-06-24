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

#include "Modules/OscillatorModule.h"
#include "Modules/AudioOutputModule.h"
#include "Modules/MathsModule.h"
#include "Modules/AudioMathsModule.h"
#include "Modules/ADSRModule.h"
#include "../Include/json.h"

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

    const char* helloWorld() {
        return "Hello World from MainComponent";
    }

private:

    void createAudioMathsModule(const json& values);
    void createMathsModule(const json& values);
    void createAudioOutputModule(const json& values);
    void createOscillatorModule(const json& values);
    void createAdsrModule(const json& values);

    std::vector<std::unique_ptr<OscillatorModule>> oscillatorModules;
    std::vector<std::unique_ptr<AudioOutputModule>> audioOutputModules;
    std::vector<std::unique_ptr<MathsModule>> mathsModules;
    std::vector<std::unique_ptr<AudioMathsModule>> audioMathsModules;    
    std::vector<std::unique_ptr<ADSRModule>> adsrModules;    

    std::unordered_map<int, float> cvParamLookup;   // might have to make this a buffer/array so we can do maths per sample 
    std::unordered_map<int, juce::AudioBuffer<float>> audioLookup;

    bool modulesCreated;
    bool shouldStop = false;
    double sampleRate;
    int samplesPerBlockExpected;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
