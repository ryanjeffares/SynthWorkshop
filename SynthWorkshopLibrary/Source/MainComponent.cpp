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
MainComponent::MainComponent()
    : modulesCreated(false), shouldStop(false), moduleCreationCanProceed(false), firstRun(true), masterVolume(0.1f)
{        
    setAudioChannels(0, 2);
    
    mathsFunctionLookup = 
    {
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

MainComponent::~MainComponent() 
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int spbe, double sr) 
{
    sampleRate = sr;
    samplesPerBlockExpected = spbe;
}

void MainComponent::releaseResources() 
{
    for (auto& module : processorModules) 
    {
        module->releaseResources();
    }
    for (auto& io : audioOutputModules) 
    {
        io->releaseResources();
    }
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) 
{    
    if (!modulesCreated.load())
    {
        if (!firstRun.load() && !threadNotified) 
        {
            std::unique_lock<std::mutex> lock(mtx);
            moduleCreationCanProceed.store(true);
            cv.notify_all();
            threadNotified = true;
        }
        return;
    }

    if (shouldStop.load()) return;
    
    int numSamples = bufferToFill.buffer->getNumSamples();
    int numChannels = bufferToFill.buffer->getNumChannels();
    
    for (auto& mod : processorModules) 
    {
        mod->getNextAudioBlock(numSamples, numChannels);
    }
    
    for (auto& audioOut : audioOutputModules) 
    {
        audioOut->getNextAudioBlock(bufferToFill);
    }
    
    bufferToFill.buffer->applyGain(masterVolume);
}

void MainComponent::setCvParam(int index, float value) 
{
    if (cvParamLookup.find(index) != cvParamLookup.end()) 
    {
        for (int i = 0; i < cvParamLookup[index].size(); i++)
        {
            cvParamLookup[index][i] = value;
        }
    }
}

float MainComponent::getCvParam(int index) 
{
    if (cvParamLookup.find(index) != cvParamLookup.end()) 
    {
        return cvParamLookup[index][0];
    }
    else 
    {
        return 0;
    }
}

void MainComponent::setMasterVolume(float value) 
{
    masterVolume = value;
}

void MainComponent::stopAudio() 
{
    shouldStop.store(true);
    
    for (auto& module : processorModules) 
    {
        module->setReady(false);
    }
    for (auto& io : audioOutputModules) 
    {
        io->setReady(false);
    }
}

bool MainComponent::createModulesFromJson(const char* jsonText) 
{        
    stopAudio();
    modulesCreated.store(false);
    
    try 
    {           
        if (!firstRun.load())
        {
            std::unique_lock<std::mutex> lock(mtx);
            while (!moduleCreationCanProceed.load())
            {
                cv.wait(lock);
            }
        }
        
        processorModules.clear();
        audioOutputModules.clear();
        cvParamLookup.clear();
        audioLookup.clear();

        auto j = json::parse(jsonText);
        
        for (const auto& moduleType : j.items()) 
        {    
            const auto& modType = moduleType.key();
            for (const auto& mod : moduleType.value().items()) 
            {
                if (modType == "oscillator_modules") 
                {
                    createOscillatorModule(mod.value());
                }
                else if (modType == "io_modules") 
                {
                    createAudioOutputModule(mod.value());
                }
                else if (modType == "maths_modules") 
                {
                    const auto& values = mod.value();
                    const auto& type = values["type_string"].get<std::string>();
                    if (type == "CV") 
                    {
                        createMathsModule(values);
                    }
                    else if (type == "Audio") 
                    {
                        createAudioMathsModule(values);
                    }
                }
                else if (modType == "adsr_modules") 
                {
                    createAdsrModule(mod.value());
                }
                else if (modType == "number_boxes") 
                {
                    // all we need to do if we see a number box is set the initial
                    const auto& values = mod.value();
                    float initial = 0;
                    if (values.contains("initial_value")) 
                    {
                        initial = values["initial_value"].get<float>();
                        if (values.contains("output_to")) 
                        {
                            auto idx = values["output_to"].get<int>();
                            if (cvParamLookup.find(idx) == cvParamLookup.end()) 
                            {
                                std::vector<float> temp(samplesPerBlockExpected, initial);
                                cvParamLookup.emplace(idx, std::move(temp));
                            }
                            else 
                            {
                                std::fill(cvParamLookup[idx].begin(), cvParamLookup[idx].end(), initial);
                            }
                        }
                    }                    
                }
            }
        }
         
        for (auto& io : audioOutputModules) 
        {
            io->prepareToPlay(samplesPerBlockExpected, sampleRate);
            io->setReady(true);
        }
        
        for (auto& module : processorModules) 
        {
            module->prepareToPlay(samplesPerBlockExpected, sampleRate);
            module->setReady(true);
        }

        modulesCreated.store(true);
        shouldStop.store(false);
        moduleCreationCanProceed.store(false);
        firstRun.store(false);
        threadNotified = false;
        
        return true;
    }
    catch (const std::exception& e) 
    {        
        return false;
    }    
}

void MainComponent::createAudioMathsModule(const json& values) 
{    
    MathsModuleType mathsType = (MathsModuleType)values["type_int"].get<int>();
    AudioCV audioCv = values["incoming_signal_type"].get<std::string>() == "Audio" ? AudioCV::Audio : AudioCV::CV;

    std::vector<int> leftInputs;
    
    if (values.contains("left_input_from")) 
    {
        leftInputs = values["left_input_from"].get<std::vector<int>>();
    }

    std::vector<int> rightInputs;
    if (values.contains("right_input_from")) 
    {
        rightInputs = values["right_input_from"].get<std::vector<int>>();
    }

    int outputId = -1;
    if (values.contains("output_id")) 
    {
        outputId = values["output_id"].get<int>();
        juce::AudioBuffer<float> buff(2, samplesPerBlockExpected);
        audioLookup.emplace(outputId, buff);
    }
    
    auto mathsModule = std::make_unique<AudioMathsModule>(cvParamLookup, audioLookup, leftInputs, rightInputs, outputId, audioCv, mathsFunctionLookup[mathsType]);
    processorModules.push_back(std::move(mathsModule));
}

void MainComponent::createMathsModule(const json& values) 
{
    MathsModuleType mathsType = (MathsModuleType)values["type_int"].get<int>();

    std::vector<int> leftIn;
    if (values.contains("left_input_from")) 
    {
        leftIn = values["left_input_from"].get<std::vector<int>>();
        for (auto i : leftIn) 
        {
            if (cvParamLookup.find(i) == cvParamLookup.end()) 
            {
                std::vector<float> temp(samplesPerBlockExpected, 1);
                cvParamLookup.emplace(i, std::move(temp));
            }
        }
    }

    std::vector<int> rightIn;
    if (values.contains("right_input_from")) 
    {
        rightIn = values["right_input_from"].get<std::vector<int>>();
        for (auto i : rightIn) 
        {
            if (cvParamLookup.find(i) == cvParamLookup.end()) 
            {
                std::vector<float> temp(samplesPerBlockExpected, 1);
                cvParamLookup.emplace(i, std::move(temp));
            }
        }
    }

    int outputId = -1;
    if (values.contains("output_id")) 
    {
        outputId = values["output_id"].get<int>();
        if (cvParamLookup.find(outputId) == cvParamLookup.end()) 
        {
            std::vector<float> temp(samplesPerBlockExpected, 1);
            cvParamLookup.emplace(outputId, std::move(temp));
        }
    }

    if (mathsType == MathsModuleType::Map) 
    {
        int minIn = values["min_in"].get<int>();
        int maxIn = values["max_in"].get<int>();
        int minOut = values["min_out"].get<int>();
        int maxOut = values["max_out"].get<int>();
        auto mathsModule = std::make_unique<MathsModule>(cvParamLookup, leftIn, rightIn, outputId, minIn, maxIn, minOut, maxOut, mathsType);
        processorModules.push_back(std::move(mathsModule));
    }
    else 
    {
        auto mathsModule = std::make_unique<MathsModule>(cvParamLookup, leftIn, rightIn, outputId, mathsType, mathsFunctionLookup[mathsType]);
        processorModules.push_back(std::move(mathsModule));
    }
}

void MainComponent::createAudioOutputModule(const json& values) 
{
    std::vector<int> leftIndexes, rightIndexes;
    
    if (values.contains("left_input_from")) 
    {
        auto indexes = values["left_input_from"].get<std::vector<int>>();
        leftIndexes.swap(indexes);
    }
    if (values.contains("right_input_from")) 
    {
        auto indexes = values["right_input_from"].get<std::vector<int>>();
        rightIndexes.swap(indexes);
    }
    
    auto audioOut = std::make_unique<AudioOutputModule>(audioLookup, leftIndexes, rightIndexes, 2);
    audioOutputModules.push_back(std::move(audioOut));
}

void MainComponent::createOscillatorModule(const json& values) 
{    
    OscillatorType t = (OscillatorType)values["type_int"].get<int>();
    
    int freqIdx = -1;
    if (values.contains("freq_input_from")) 
    {
        freqIdx = values["freq_input_from"].get<int>();
        if (cvParamLookup.find(freqIdx) == cvParamLookup.end()) 
        {
            std::vector<float> temp(samplesPerBlockExpected, 100);
            cvParamLookup.emplace(freqIdx, std::move(temp));
        }
    }

    int soundIdx = -1;
    if (values.contains("sound_output_to")) 
    {
        soundIdx = values["sound_output_to"].get<int>();
        juce::AudioBuffer<float> buff(2, samplesPerBlockExpected);
        audioLookup.emplace(soundIdx, buff);
    }

    int pwIdx = -1;
    if (values.contains("pw_input_from")) 
    {
        pwIdx = values["pw_input_from"].get<int>();
        if (cvParamLookup.find(pwIdx) == cvParamLookup.end()) 
        {
            std::vector<float> temp(samplesPerBlockExpected, 0.5f);
            cvParamLookup.emplace(pwIdx, std::move(temp));
        }
    }

    int cvIdx = -1;
    if (values.contains("cv_out_to")) 
    {
        cvIdx = values["cv_out_to"].get<int>();
        if (cvParamLookup.find(cvIdx) == cvParamLookup.end()) 
        {
            std::vector<float> temp(samplesPerBlockExpected, 1);
            cvParamLookup.emplace(cvIdx, std::move(temp));
        }
    }

    auto osc = std::make_unique<OscillatorModule>(t, cvParamLookup, audioLookup, freqIdx, pwIdx, soundIdx, cvIdx);
    processorModules.push_back(std::move(osc));
}

void MainComponent::createAdsrModule(const json& values) 
{
    std::vector<int> inputIndexes;
    if (values.contains("input_from")) 
    {
        inputIndexes = values["input_from"].get<std::vector<int>>();
    }

    int triggerInput = -1;
    if (values.contains("trigger_input")) 
    {
        triggerInput = values["trigger_input"].get<int>();
    }

    int outputIndex = -1;
    if (values.contains("output_to")) 
    {
        outputIndex = values["output_to"].get<int>();
        juce::AudioBuffer<float> buff(2, samplesPerBlockExpected);
        audioLookup.emplace(outputIndex, buff);
    }

    // for now, these are required to exist
    int attack = values["attack_from"].get<int>();
    int decay = values["decay_from"].get<int>();
    int sustain = values["sustain_from"].get<int>();
    int release = values["release_from"].get<int>();

    auto adsr = std::make_unique<ADSRModule>(audioLookup, cvParamLookup, inputIndexes, outputIndex, 2, attack, decay, sustain, release, triggerInput);
    processorModules.push_back(std::move(adsr));
}
