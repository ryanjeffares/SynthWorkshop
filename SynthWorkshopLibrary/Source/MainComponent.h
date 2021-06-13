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

#include "Modules/OscillatorModule.h"
#include "Modules/AudioOutputModule.h"
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

    void setCvParam(int index, float value);

    const char* helloWorld() {
        return "Hello World from MainComponent";
    }

private:

    std::vector<std::shared_ptr<OscillatorModule>> oscillatorModules;
    std::vector<std::shared_ptr<AudioOutputModule>> audioOutputModules;
    std::unordered_map<int, float> cvParamLookup;
    std::unordered_map<int, juce::AudioBuffer<float>> audioLookup;

    bool modulesCreated;
    double sampleRate;
    int samplesPerBlockExpected;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
