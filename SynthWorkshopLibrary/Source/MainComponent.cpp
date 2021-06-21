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
    for (auto& audioMaths : audioMathsModules) {
        audioMaths->releaseResources();
    }
    for (auto& io : audioOutputModules) {
        io->releaseResources();
    }
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {
    if (!modulesCreated || shouldStop) return;
    for (auto& osc : oscillatorModules) {
        osc->getNextAudioBlock(bufferToFill.numSamples, bufferToFill.buffer->getNumChannels());
    }
    for (auto& maths : mathsModules) {
        maths->calculateValues();
    }
    for (auto& audioMaths : audioMathsModules) {
        audioMaths->getNextAudioBlock(bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples);
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

float MainComponent::getCvParam(int index) {
    if (cvParamLookup.find(index) != cvParamLookup.end()) {
        return cvParamLookup[index];
    }
    else {
        return 0;
    }
}

void MainComponent::stopAudio() {
    shouldStop = true;
}

bool MainComponent::createModulesFromJson(const char* jsonText) {
    modulesCreated = false;
    try {
        // TODO: Clearing these vectors is temporary, figure out a real system later.
        oscillatorModules.clear();
        audioOutputModules.clear();
        mathsModules.clear();
        audioMathsModules.clear();

        auto j = json::parse(jsonText);
        for (auto& moduleType : j.items()) {    // control, oscillator, io
            auto& modType = moduleType.key();
            for (auto& mod : moduleType.value().items()) {
                if (modType == "OscillatorModules") {
                    createOscillatorModule(mod);
                }
                else if (modType == "IOModules") {
                    createAudioOutputModule(mod);
                }
                else if (modType == "MathsModules") {
                    const auto& values = mod.value();
                    const auto& type = values["type"].get<std::string>();
                    if (type == "CV") {
                        createMathsModule(values);
                    }
                    else if (type == "Audio") {
                        createAudioMathsModule(values);
                    }
                }
                else if (modType == "NumberBoxes") {
                    const auto& values = mod.value();
                    float initial = 0;
                    if (values.contains("initialValue")) {
                        initial = values["initialValue"].get<float>();
                    }
                    if (values.contains("outputTo")) {
                        auto idx = values["outputTo"].get<int>();
                        if (cvParamLookup.find(idx) == cvParamLookup.end()) {
                            cvParamLookup.emplace(idx, initial);
                        }
                        else {
                            cvParamLookup[idx] = initial;
                        }
                    }
                }
            }
        }
        for (auto& osc : oscillatorModules) {
            osc->setReady(true);
        }
        for (auto& maths : audioMathsModules) {
            maths->setReady(true);
        }
        for (auto& io : audioOutputModules) {
            io->setReady(true);
        }
        modulesCreated = true;
        shouldStop = false;
        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
    
}

void MainComponent::createAudioMathsModule(const json& values) {
    MathsModuleType mathsType;
    AudioCV audioCv = values["IncomingSignalType"].get<std::string>() == "Audio" ? AudioCV::Audio : AudioCV::CV;

    const auto& sign = values["operator"].get<std::string>();
    if (sign == "Plus") {
        mathsType = MathsModuleType::Plus;
    }
    else if (sign == "Minus") {
        mathsType = MathsModuleType::Minus;
    }
    else if (sign == "Multiply") {
        mathsType = MathsModuleType::Multiply;
    }
    else if (sign == "Divide") {
        mathsType = MathsModuleType::Divide;
    }

    int leftIn = -1;
    if (values.contains("LeftInputFrom")) {
        leftIn = values["LeftInputFrom"].get<int>();
    }
    int rightIn = -1;
    if (values.contains("RightInputFrom")) {
        rightIn = values["RightInputFrom"].get<int>();
    }
    int outputId = -1;
    if (values.contains("outputId")) {
        outputId = values["outputId"].get<int>();
        juce::AudioBuffer<float> buff(2, samplesPerBlockExpected);
        audioLookup.emplace(outputId, buff);
    }
    auto mathsModule = std::make_unique<AudioMathsModule>(cvParamLookup, audioLookup, leftIn, rightIn, outputId, mathsType, audioCv);
    mathsModule->prepareToPlay(samplesPerBlockExpected, sampleRate);
    audioMathsModules.push_back(std::move(mathsModule));
}

void MainComponent::createMathsModule(const json& values) {
    MathsModuleType mathsType;
    const auto& sign = values["operator"].get<std::string>();
    if (sign == "Plus") {
        mathsType = MathsModuleType::Plus;
    }
    else if (sign == "Minus") {
        mathsType = MathsModuleType::Minus;
    }
    else if (sign == "Multiply") {
        mathsType = MathsModuleType::Multiply;
    }
    else if (sign == "Divide") {
        mathsType = MathsModuleType::Divide;
    }

    int leftIn = -1;
    if (values.contains("LeftInputFrom")) {
        leftIn = values["LeftInputFrom"].get<int>();
        if (cvParamLookup.find(leftIn) == cvParamLookup.end()) {
            cvParamLookup.emplace(leftIn, 1);
        }
    }
    int rightIn = -1;
    if (values.contains("RightInputFrom")) {
        rightIn = values["RightInputFrom"].get<int>();
        if (cvParamLookup.find(rightIn) == cvParamLookup.end()) {
            cvParamLookup.emplace(rightIn, 1);
        }
    }
    int outputId = -1;
    if (values.contains("outputId")) {
        outputId = values["outputId"].get<int>();
        if (cvParamLookup.find(outputId) == cvParamLookup.end()) {
            cvParamLookup.emplace(outputId, 1);
        }
    }

    auto mathsModule = std::make_unique<MathsModule>(cvParamLookup, leftIn, rightIn, outputId, mathsType);
    mathsModules.push_back(std::move(mathsModule));
}

void MainComponent::createAudioOutputModule(const detail::iteration_proxy_value<detail::iter_impl<nlohmann::json>>& mod) {
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
    auto audioOut = std::make_unique<AudioOutputModule>(audioLookup, leftIndexes, rightIndexes, 2);
    audioOut->prepareToPlay(samplesPerBlockExpected, sampleRate);
    audioOutputModules.push_back(std::move(audioOut));
}

void MainComponent::createOscillatorModule(const detail::iteration_proxy_value<detail::iter_impl<nlohmann::json>>& mod) {
    const auto& values = mod.value();
    const auto& typeStr = values["type"].get<std::string>();
    OscillatorType t;
    if (typeStr == "Saw") {
        t = OscillatorType::Saw;
    }
    else if (typeStr == "Pulse") {
        t = OscillatorType::Pulse;
    }
    else if (typeStr == "Sine") {
        t = OscillatorType::Sine;
    }
    else if (typeStr == "Tri") {
        t = OscillatorType::Tri;
    }
    int freqIdx = -1;
    if (values.contains("FreqInputFrom")) {
        freqIdx = values["FreqInputFrom"].get<int>();
        if (cvParamLookup.find(freqIdx) == cvParamLookup.end()) {
            cvParamLookup.emplace(freqIdx, 100);
        }
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
        if (cvParamLookup.find(pwIdx) == cvParamLookup.end()) {
            cvParamLookup.emplace(pwIdx, 0.5f);
        }
    }
    int rmIdx = -1;
    if (values.contains("RMInputFrom")) {
        rmIdx = values["RMInputFrom"].get<int>();
        if (cvParamLookup.find(rmIdx) == cvParamLookup.end()) {
            cvParamLookup.emplace(rmIdx, 1);
        }
    }

    int cvIdx = -1;
    if (values.contains("CVOutTo")) {
        cvIdx = values["CVOutTo"].get<int>();
        if (cvParamLookup.find(cvIdx) == cvParamLookup.end()) {
            cvParamLookup.emplace(cvIdx, 1);
        }
    }

    auto osc = std::make_unique<OscillatorModule>(t, cvParamLookup, audioLookup, freqIdx, pwIdx, rmIdx, soundIdx, cvIdx);
    osc->prepareToPlay(samplesPerBlockExpected, sampleRate);
    oscillatorModules.push_back(std::move(osc));
}
