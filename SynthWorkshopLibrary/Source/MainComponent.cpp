/*
  ==============================================================================

    MainComponent.cpp
    Created: 12 Jun 2021 7:00:37pm
    Author:  ryand

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainComponent.h"

using namespace nlohmann;

//==============================================================================
MainComponent::MainComponent() : modulesCreated(false) {
    setAudioChannels(0, 2);
}

MainComponent::~MainComponent() {
    shutdownAudio();
}

void MainComponent::prepareToPlay(int spbe, double sr) {
    sampleRate = sr;
    samplesPerBlockExpected = spbe;
}

void MainComponent::releaseResources() {
    for (auto& osc : oscillatorModules) {
        osc->releaseResources();
    }
    for (auto& io : audioOutputModules) {
        io->releaseResources();
    }
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    if (!modulesCreated) return;
    for (auto& buff : audioLookup) {
        buff.second.clear();
    }
    for (auto& osc : oscillatorModules) {
        osc->getNextAudioBlock(bufferToFill.numSamples, bufferToFill.buffer->getNumChannels());
    }
    for (auto& audioOut : audioOutputModules) {
        audioOut->getNextAudioBlock(bufferToFill);
    }
}

void MainComponent::setCvParam(int index, float value) {
    if (cvParamLookup.find(index) != cvParamLookup.end()) {
        cvParamLookup[index] = value;
    }
}

bool MainComponent::createModulesFromJson(const char* jsonText) {
    modulesCreated = false;
    try {
        // TODO: Clearing these vectors is temporary, figure out a real system later.
        oscillatorModules.clear();
        audioOutputModules.clear();
        std::string jsonString(jsonText);
        auto j = json::parse(jsonText);
        for (auto& moduleType : j.items()) {    // control, oscillator, io
            for (auto& mod : moduleType.value().items()) {
                auto& modType = moduleType.key();
                if (modType == "ControlModules") {

                }
                else if (modType == "OscillatorModules") {
                    const auto& values = mod.value();
                    const auto& typeStr = values["type"].get<std::string>();
                    OscillatorModule::OscillatorType t;
                    if (typeStr == "Saw") {
                        t = OscillatorModule::OscillatorType::Saw;
                    }
                    else if (typeStr == "Pulse") {
                        t = OscillatorModule::OscillatorType::Pulse;
                    }
                    else if (typeStr == "Sine") {
                        t = OscillatorModule::OscillatorType::Sine;
                    }
                    else if (typeStr == "Tri") {
                        t = OscillatorModule::OscillatorType::Tri;
                    }
                    int freqIdx = -1; 
                    if (values.contains("FreqInputFrom")) {
                        freqIdx = values["FreqInputFrom"].get<int>();
                        cvParamLookup.emplace(freqIdx, 100);
                    }
                    int soundIdx = -1; 
                    if (values.contains("SoundOutputTo")) {
                        soundIdx = values["SoundOutputTo"].get<int>();
                        juce::AudioBuffer<float> buff(2, samplesPerBlockExpected);
                        audioLookup.emplace(soundIdx, buff);
                    }
                    int pwIdx = -1;
                    if (values.contains("PWInputFrom")) {
                        pwIdx = values["PWInputFrom"].get<int>();
                        cvParamLookup.emplace(pwIdx, 0.5f);
                    }
                    int fmIdx = -1; 
                    if (values.contains("FMInputFrom")) {
                        fmIdx = values["FMInputFrom"].get<int>();
                        cvParamLookup.emplace(fmIdx, 1);
                    }

                    // TODO: CV Output
                    int cvIdx = values.contains("CVOutTo") ? values["CVOutTo"].get<int>() : -1;

                    auto osc = std::make_shared<OscillatorModule>(t, cvParamLookup, audioLookup, freqIdx, pwIdx, fmIdx, soundIdx, cvIdx);
                    osc->prepareToPlay(samplesPerBlockExpected, sampleRate);
                    oscillatorModules.push_back(osc);
                }
                else if (modType == "IOModules") {
                    const auto& values = mod.value();
                    std::vector<int> leftIndexes, rightIndexes;
                    if (values.contains("LeftInputFrom")) {
                        auto indexes = values["LeftInputFrom"].get<std::vector<int>>();
                        leftIndexes.swap(indexes);
                    }
                    if (values.contains("RightInputFrom")) {
                        auto indexes = values["RightInputFrom"].get<std::vector<int>>();
                        rightIndexes.swap(indexes);
                    }
                    auto audioOut = std::make_shared<AudioOutputModule>(audioLookup, leftIndexes, rightIndexes, 2);
                    audioOut->prepareToPlay(samplesPerBlockExpected, sampleRate);
                    audioOutputModules.push_back(audioOut);
                }
            }
        }
        modulesCreated = true;
        for (auto& osc : oscillatorModules) {
            osc->setReady(true);
        }
        for (auto& io : audioOutputModules) {
            io->setReady(true);
        }
        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
    
}
