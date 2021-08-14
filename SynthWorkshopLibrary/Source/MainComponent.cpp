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
    mathsFunctionLookup = {
        { MathsModuleType::Plus, [](float left, float right) {return left + right; } },
        { MathsModuleType::Minus,[](float left, float right) {return left - right; }},
        { MathsModuleType::Multiply,[](float left, float right) {return left * right; }},
        { MathsModuleType::Divide,[](float left, float right) {return left / right; }},
        { MathsModuleType::Mod,[](float left, float right) {return std::fmod(left, right); }},
        { MathsModuleType::Sin, [](float left, float right) {return std::sin(left); }},
        { MathsModuleType::Cos, [](float left, float right) {return std::cos(left); }},
        { MathsModuleType::Tan, [](float left, float right) {return std::tan(left); }},
        { MathsModuleType::Asin, [](float left, float right) {return std::asin(left); }},
        { MathsModuleType::Acos, [](float left, float right) {return std::acos(left); }},
        { MathsModuleType::Atan, [](float left, float right) {return std::atan(left); }},
        { MathsModuleType::Abs, [](float left, float right) {return std::fabs(left); }},
        { MathsModuleType::Exp, [](float left, float right) {return std::pow(left, right); }},
        { MathsModuleType::Int, [](float left, float right) {return (int)left; }},
        { MathsModuleType::Mtof, [](float left, float right) {return std::powf(2, ((int)left - 69) / 12.f) * 440.f; }},
        { MathsModuleType::Ftom, [](float left, float right) {return (int)(69 + (12 * std::log2f(left / 440.f))); }}
    };
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
    for (auto& adsr : adsrModules) {
        adsr->getNextAudioBlock();
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
    for (auto& osc : oscillatorModules) {
        osc->setReady(false);
    }
    for (auto& maths : audioMathsModules) {
        maths->setReady(false);
    }
    for (auto& io : audioOutputModules) {
        io->setReady(false);
    }
    for (auto& adsr : adsrModules) {
        adsr->setReady(false);
    }
}

bool MainComponent::createModulesFromJson(const char* jsonText) {
    modulesCreated = false;
    try {
        // TODO: Clearing these vectors is temporary, figure out a real system later.
        oscillatorModules.clear();
        audioOutputModules.clear();
        mathsModules.clear();
        audioMathsModules.clear();
        adsrModules.clear();
        cvParamLookup.clear();
        audioLookup.clear();


        auto j = json::parse(jsonText);
        for (auto& moduleType : j.items()) {    // control, oscillator, io
            auto& modType = moduleType.key();
            for (auto& mod : moduleType.value().items()) {
                if (modType == "OscillatorModules") {
                    createOscillatorModule(mod.value());
                }
                else if (modType == "IOModules") {
                    createAudioOutputModule(mod.value());
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
                else if (modType == "ADSRModules") {
                    createAdsrModule(mod.value());
                }
                else if (modType == "NumberBoxes") {
                    // all we need to do if we see a number box is set the initial
                    const auto& values = mod.value();
                    float initial = 0;
                    if (values.contains("initialValue")) {
                        initial = values["initialValue"].get<float>();
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
        for (auto& adsr : adsrModules) {
            adsr->setReady(true);
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
    else if (sign == "Sin") {
        mathsType = MathsModuleType::Sin;
    }
    else if (sign == "Cos") {
        mathsType = MathsModuleType::Cos;
    }
    else if (sign == "Tan") {
        mathsType = MathsModuleType::Tan;
    }
    else if (sign == "Asin") {
        mathsType = MathsModuleType::Asin;
    }
    else if (sign == "Acos") {
        mathsType = MathsModuleType::Acos;
    }
    else if (sign == "Atan") {
        mathsType = MathsModuleType::Atan;
    }
    else if (sign == "Abs") {
        mathsType = MathsModuleType::Abs;
    }
    else if (sign == "Exp") {
        mathsType = MathsModuleType::Exp;
    }
    else {
        mathsType = MathsModuleType::Plus;
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
    auto mathsModule = std::make_unique<AudioMathsModule>(cvParamLookup, audioLookup, leftIn, rightIn, outputId, audioCv, mathsFunctionLookup[mathsType]);
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
    else if (sign == "Mod") {
        mathsType = MathsModuleType::Mod;
    }
    else if (sign == "Sin") {
        mathsType = MathsModuleType::Sin;
    }
    else if (sign == "Cos") {
        mathsType = MathsModuleType::Cos;
    }
    else if (sign == "Tan") {
        mathsType = MathsModuleType::Tan;
    }
    else if (sign == "Asin") {
        mathsType = MathsModuleType::Asin;
    }
    else if (sign == "Acos") {
        mathsType = MathsModuleType::Acos;
    }
    else if (sign == "Atan") {
        mathsType = MathsModuleType::Atan;
    }
    else if (sign == "Abs") {
        mathsType = MathsModuleType::Abs;
    }
    else if (sign == "Exp") {
        mathsType = MathsModuleType::Exp;
    }
    else if (sign == "Int") {
        mathsType = MathsModuleType::Int;
    }
    else if (sign == "Map") {
        mathsType = MathsModuleType::Map;
    }
    else if (sign == "Mtof") {
        mathsType = MathsModuleType::Mtof;
    }
    else if (sign == "Ftom") {
        mathsType = MathsModuleType::Ftom;
    }
    else {
        mathsType = MathsModuleType::Plus;
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

    if (mathsType == MathsModuleType::Map) {
        int minIn = values["minIn"].get<int>();
        int maxIn = values["maxIn"].get<int>();
        int minOut = values["minOut"].get<int>();
        int maxOut = values["maxOut"].get<int>();
        auto mathsModule = std::make_unique<MathsModule>(cvParamLookup, leftIn, rightIn, outputId, minIn, maxIn, minOut, maxOut, mathsType);
        mathsModules.push_back(std::move(mathsModule));
    }
    else {
        auto mathsModule = std::make_unique<MathsModule>(cvParamLookup, leftIn, rightIn, outputId, mathsType, mathsFunctionLookup[mathsType]);
        mathsModules.push_back(std::move(mathsModule));
    }
}

void MainComponent::createAudioOutputModule(const json& values) {
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

void MainComponent::createOscillatorModule(const json& values) {
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

    int cvIdx = -1;
    if (values.contains("CVOutTo")) {
        cvIdx = values["CVOutTo"].get<int>();
        if (cvParamLookup.find(cvIdx) == cvParamLookup.end()) {
            cvParamLookup.emplace(cvIdx, 1);
        }
    }

    auto osc = std::make_unique<OscillatorModule>(t, cvParamLookup, audioLookup, freqIdx, pwIdx, soundIdx, cvIdx);
    osc->prepareToPlay(samplesPerBlockExpected, sampleRate);
    oscillatorModules.push_back(std::move(osc));
}

void MainComponent::createAdsrModule(const json& values) {
    std::vector<int> inputIndexes;
    if (values.contains("inputFrom")) {
        inputIndexes = values["inputFrom"].get<std::vector<int>>();
    }
    int triggerInput = -1;
    if (values.contains("triggerInput")) {
        triggerInput = values["triggerInput"].get<int>();
    }
    int outputIndex = -1;
    if (values.contains("outputTo")) {
        outputIndex = values["outputTo"].get<int>();
        juce::AudioBuffer<float> buff(2, samplesPerBlockExpected);
        audioLookup.emplace(outputIndex, buff);
    }
    // for now, these are required to exist
    int attack = values["attackFrom"].get<int>();
    int decay = values["decayFrom"].get<int>();
    int sustain = values["sustainFrom"].get<int>();
    int release = values["releaseFrom"].get<int>();

    auto adsr = std::make_unique<ADSRModule>(audioLookup, cvParamLookup, inputIndexes, outputIndex, 2, attack, decay, sustain, release, triggerInput);
    adsr->prepareToPlay(samplesPerBlockExpected, sampleRate);
    adsrModules.push_back(std::move(adsr));
}
