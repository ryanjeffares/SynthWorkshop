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
using namespace SynthWorkshop::Modules;
using namespace SynthWorkshop::Enums;

//==============================================================================
MainComponent::MainComponent()
    : m_ShouldStopAudio(false), 
    m_NewProcessorModuleCreated(false), 
    m_NewOutputModuleCreated(false), 
    m_NewTriggerModuleCreated(false), 
    m_ShouldDestroyOutputModule(false), 
    m_ShouldDestroyProcessorModule(false), 
    m_MasterVolume(0.1f)
{        
    setAudioChannels(0, 2);
    
    m_MathsFunctionLookup = 
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
    m_SampleRate = sr;
    m_SamplesPerBlockExpected = spbe;
}

void MainComponent::releaseResources() 
{
    for (auto& module : m_ProcessorModules) 
    {
        module->releaseResources();
    }
    for (auto& io : m_AudioOutputModules) 
    {
        io->releaseResources();
    }
    for (auto& t : m_TriggerModules)
    {
        t->releaseResources();
    }
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) 
{    
    for (auto& buff : m_AudioLookup)
    {
        buff.second.clear();
    }

    checkForUpdates();

    if (m_ShouldStopAudio.load()) return;
    
    int numSamples = bufferToFill.buffer->getNumSamples();
    int numChannels = bufferToFill.buffer->getNumChannels();

    for (auto& trig : m_TriggerModules)
    {
        if (auto processor = dynamic_cast<ProcessorModule*>(trig.get()))
        {
            processor->getNextAudioBlock(numSamples, numChannels);
        }
    }
    
    for (auto& mod : m_ProcessorModules) 
    {
        mod->getNextAudioBlock(numSamples, numChannels);
    }
    
    for (auto& audioOut : m_AudioOutputModules) 
    {
        audioOut->getNextAudioBlock(bufferToFill);
    }
    
    bufferToFill.buffer->applyGain(m_MasterVolume);
}

// should only be called from the audio thread
void MainComponent::checkForUpdates()
{
    // this is a temp hacky fix to try get rid of crashes on shutdown
    if (m_ShouldClearModules.load())
    {
        juce::ScopedLock sl(m_Cs);
            
        m_AudioOutputModules.clear();
        m_ProcessorModules.clear();
        m_TriggerModules.clear();

        m_ShouldDestroyOutputModule.store(false);
        m_ShouldDestroyProcessorModule.store(false);
        m_ShouldDestroyTriggerModule.store(false);

        m_NewOutputModuleCreated.store(false);
        m_NewProcessorModuleCreated.store(false);
        m_NewTriggerModuleCreated.store(false);

        m_ShouldClearModules.store(false);

        return;
    }

    // erase the any desired modules
    if (m_ShouldDestroyOutputModule.load())
    {
        juce::ScopedLock sl(m_Cs);

        auto it = std::find_if(
            m_AudioOutputModules.begin(),
            m_AudioOutputModules.end(),
            [&](const std::unique_ptr<OutputModule>& mod)
            {
                return mod->getModuleId() == m_ModuleIdToDestroy;
            }
        );

        if (it != m_AudioOutputModules.end())
        {
            m_AudioOutputModules.erase(it);
        }

        m_ShouldDestroyOutputModule.store(false);
    }

    if (m_ShouldDestroyTriggerModule.load())
    {
        juce::ScopedLock sl(m_Cs);

        auto it = std::find_if(
            m_TriggerModules.begin(),
            m_TriggerModules.end(),
            [&](const std::unique_ptr<Triggerable>& mod)
            {
                return mod->getModuleId() == m_ModuleIdToDestroy;
            }
        );

        if (it != m_TriggerModules.end())
        {
            m_TriggerModules.erase(it);
        }

        m_ShouldDestroyTriggerModule.store(true);
    }

    if (m_ShouldDestroyProcessorModule.load())
    {
        juce::ScopedLock sl(m_Cs);

        auto it = std::find_if(
            m_ProcessorModules.begin(),
            m_ProcessorModules.end(),
            [&](const std::unique_ptr<ProcessorModule>& mod)
            {
                return mod->getModuleId() == m_ModuleIdToDestroy;
            }
        );

        if (it!= m_ProcessorModules.end())
        {
            m_ProcessorModules.erase(it);
        }

        m_ShouldDestroyProcessorModule.store(false);
    }

    // take control of any allocated modules in a unique ptr and push into vector
    if (m_NewProcessorModuleCreated.load())
    {
        juce::ScopedLock sl(m_Cs);

        std::unique_ptr<ProcessorModule> p(m_LastCreatedProcessorModule);
        m_ProcessorModules.push_back(std::move(p));
        m_LastCreatedProcessorModule = nullptr;
        m_NewProcessorModuleCreated.store(false);
    }

    if (m_NewOutputModuleCreated.load())
    {
        juce::ScopedLock sl(m_Cs);

        std::unique_ptr<OutputModule> p(m_LastCreatedOutputModule);
        m_AudioOutputModules.push_back(std::move(p));
        m_LastCreatedProcessorModule = nullptr;
        m_NewOutputModuleCreated.store(false);
    }

    if (m_NewTriggerModuleCreated.load())
    {
        juce::ScopedLock sl(m_Cs);

        std::unique_ptr<Triggerable> t(m_LastCreatedTriggerModule);
        m_TriggerModules.push_back(std::move(t));
        m_LastCreatedTriggerModule = nullptr;
        m_NewTriggerModuleCreated.store(false);
    }
}

void MainComponent::setCvParam(int index, float value) 
{
    if (m_CvParamLookup.find(index) != m_CvParamLookup.end()) 
    {
        for (int i = 0; i < m_CvParamLookup[index].size(); i++)
        {
            m_CvParamLookup[index][i] = value;
        }
    }
}

float MainComponent::getCvParam(int index) 
{
    if (m_CvParamLookup.find(index) != m_CvParamLookup.end()) 
    {
        return m_CvParamLookup[index][0];
    }

    return 0;
}

bool MainComponent::getTriggerableState(int moduleId)
{
    auto it = std::find_if(
        m_TriggerModules.begin(),
        m_TriggerModules.end(),
        [moduleId](const std::unique_ptr<Triggerable>& mod)
        {
            return mod->getModuleId() == moduleId;
        }
    );

    if (it != m_TriggerModules.end())
    {
        return (*it)->getState();
    }

    return false;
}

void MainComponent::triggerCallback(int moduleId, bool state)
{
    auto it = std::find_if(
        m_TriggerModules.begin(),
        m_TriggerModules.end(),
        [moduleId](const std::unique_ptr<Triggerable>& mod)
        {
            return mod->getModuleId() == moduleId;
        }
    );

    if (it != m_TriggerModules.end())
    {
        (*it)->triggerCallback(it->get(), state);
    }
}

void MainComponent::createCvBufferWithKey(int key)
{
    if (m_CvParamLookup.find(key) == m_CvParamLookup.end())
    {
        m_CvParamLookup.emplace(key, std::vector<float>(m_SamplesPerBlockExpected, 0));
    }
}

void MainComponent::createCvBufferWithKey(int key, float value)
{
    if (m_CvParamLookup.find(key) == m_CvParamLookup.end())
    {
        m_CvParamLookup.emplace(key, std::vector<float>(m_SamplesPerBlockExpected, value));
    }
}

bool MainComponent::setModuleInputIndex(bool audioModule, bool add, int moduleId, int outputIndex, int targetIndex)
{
    if (audioModule)
    {
        auto it = std::find_if(
            m_AudioOutputModules.begin(),
            m_AudioOutputModules.end(),
            [moduleId](const std::unique_ptr<OutputModule>& mod)
            {
                return mod->getModuleId() == moduleId;
            }
        );
        
        if (it != m_AudioOutputModules.end())
        {
            if (add)
                (*it)->setInputIndex(outputIndex, targetIndex);
            else
                (*it)->removeInputIndex(outputIndex, targetIndex);

            return true;
        }

        return false;
    }
    else
    {
        auto it = std::find_if(
            m_ProcessorModules.begin(),
            m_ProcessorModules.end(),
            [moduleId](const std::unique_ptr<ProcessorModule>& mod)
            {
                return mod->getModuleId() == moduleId;
            }
        );

        if (it != m_ProcessorModules.end())
        {
            if (add)
                (*it)->setInputIndex(outputIndex, targetIndex);
            else
                (*it)->removeInputIndex(outputIndex, targetIndex);

            return true;
        }

        return false;
    }
}

bool MainComponent::setTriggerTarget(bool add, int senderId, int targetId)
{
    if (add)
    {
        return addTriggerTarget(senderId, targetId);
    }
    else
    {
        return removeTriggerTarget(senderId, targetId);
    }
}

bool MainComponent::addTriggerTarget(int senderId, int targetId)
{
    auto senderIt = std::find_if(
        m_TriggerModules.begin(),
        m_TriggerModules.end(),
        [senderId](const std::unique_ptr<Triggerable>& mod)
        {
            return mod->getModuleId() == senderId;
        }
    );

    if (senderIt != m_TriggerModules.end())
    {
        auto targetIt = std::find_if(
            m_TriggerModules.begin(),
            m_TriggerModules.end(),
            [targetId](const std::unique_ptr<Triggerable>& mod)
            {
                return mod->getModuleId() == targetId;
            }
        );

        if (targetIt != m_TriggerModules.end())
        {
            (*senderIt)->addTriggerTarget(targetIt->get());
            return true;
        }

        // could be a processor module
        auto targetProcessorIt = std::find_if(
            m_ProcessorModules.begin(),
            m_ProcessorModules.end(),
            [targetId](const std::unique_ptr<ProcessorModule>& mod)
            {
                return mod->getModuleId() == targetId;
            }
        );

        if (targetProcessorIt != m_ProcessorModules.end())
        {
            if (auto t = dynamic_cast<Triggerable*>(targetProcessorIt->get()))
            {
                (*senderIt)->addTriggerTarget(t);
                return true;
            }
        }
    }
    else
    {
        auto processorSenderIt = std::find_if(
            m_ProcessorModules.begin(),
            m_ProcessorModules.end(),
            [senderId](const std::unique_ptr<ProcessorModule>& mod)
            {
                return mod->getModuleId() == senderId;
            }
        );

        if (processorSenderIt != m_ProcessorModules.end())
        {
            auto targetIt = std::find_if(
                m_TriggerModules.begin(),
                m_TriggerModules.end(),
                [targetId](const std::unique_ptr<Triggerable>& mod)
                {
                    return mod->getModuleId() == targetId;
                }
            );

            if (targetIt != m_TriggerModules.end())
            {
                if (auto t = dynamic_cast<Triggerable*>(processorSenderIt->get()))
                {
                    t->addTriggerTarget(targetIt->get());
                    return true;
                }
            }

            // could be a processor module
            auto targetProcessorIt = std::find_if(
                m_ProcessorModules.begin(),
                m_ProcessorModules.end(),
                [targetId](const std::unique_ptr<ProcessorModule>& mod)
                {
                    return mod->getModuleId() == targetId;
                }
            );

            if (targetProcessorIt != m_ProcessorModules.end())
            {
                if (auto targetT = dynamic_cast<Triggerable*>(targetProcessorIt->get()))
                {
                    if (auto senderT = dynamic_cast<Triggerable*>(processorSenderIt->get()))
                    {
                        senderT->addTriggerTarget(targetT);
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool MainComponent::removeTriggerTarget(int senderId, int targetId)
{
    auto it = std::find_if(
        m_TriggerModules.begin(),
        m_TriggerModules.end(),
        [senderId](const std::unique_ptr<Triggerable>& trig)
        {
            return trig->getModuleId() == senderId;
        }
    );

    if (it != m_TriggerModules.end())
    {
        (*it)->removeTriggerTarget(targetId);
        return true;
    }

    auto processorIt = std::find_if(
        m_ProcessorModules.begin(),
        m_ProcessorModules.end(),
        [senderId](const std::unique_ptr<ProcessorModule>& mod)
        {
            return mod->getModuleId() == senderId;
        }
    );

    if (processorIt != m_ProcessorModules.end())
    {
        if (auto t = dynamic_cast<Triggerable*>(processorIt->get()))
        {
            t->removeTriggerTarget(targetId);
            return true;
        }
    }

    return false;
}

void MainComponent::setAudioMathsIncomingSignal(int moduleId, int type)
{
    auto it = std::find_if(
        m_ProcessorModules.begin(),
        m_ProcessorModules.end(),
        [moduleId](const std::unique_ptr<ProcessorModule>& mod)
        {
            return mod->getModuleId() == moduleId;
        }
    );

    if (it != m_ProcessorModules.end())
    {
        if (auto mathsModule = dynamic_cast<AudioMathsModule*>(it->get()))
        {
            mathsModule->setIncomingSignalType((AudioCV)type);
        }
    }
}

void MainComponent::setNumberBoxValue(int moduleId, float value)
{
    auto it = std::find_if(
        m_ProcessorModules.begin(),
        m_ProcessorModules.end(),
        [moduleId](const std::unique_ptr<ProcessorModule>& mod)
        {
            return mod->getModuleId() == moduleId;
        }
    );

    if (it != m_ProcessorModules.end())
    {
        if (auto mod = dynamic_cast<NumberBoxModule*>(it->get()))
        {
            mod->setValue(value);
        }
    }
}

bool MainComponent::setSoundfileModuleSample(int moduleId, const char* filePath)
{
    auto it = std::find_if(
        m_ProcessorModules.begin(),
        m_ProcessorModules.end(),
        [moduleId](const std::unique_ptr<ProcessorModule>& mod)
        {
            return mod->getModuleId() == moduleId;
        }
    );

    if (it != m_ProcessorModules.end())
    {
        if (auto sf = dynamic_cast<SoundfileModule*>(it->get()))
        {
            juce::String p(filePath);
            sf->setSample(p);
            return true;
        }
    }

    return false;
}

void MainComponent::setMasterVolume(float value) 
{
    m_MasterVolume = value;
}

void MainComponent::stopAudio() 
{
    m_ShouldStopAudio.store(true);
    
    for (auto& module : m_ProcessorModules) 
    {
        module->setReady(false);
    }
    for (auto& io : m_AudioOutputModules) 
    {
        io->setReady(false);
    }
}

void MainComponent::resumeAudio()
{
    for (auto& module : m_ProcessorModules)
    {
        module->setReady(true);
    }
    for (auto& io : m_AudioOutputModules)
    {
        io->setReady(true);
    }

    m_ShouldStopAudio.store(false);
}

bool MainComponent::createAudioMathsModule(const char* json)
{
    try
    {
        auto values = json::parse(json);
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
            juce::AudioBuffer<float> buff(2, m_SamplesPerBlockExpected);
            m_AudioLookup.emplace(outputId, buff);
        }

        int id = values["global_index"].get<int>();
        float init = values["initial_value"].get<float>();

        m_LastCreatedProcessorModule = new AudioMathsModule(
            m_CvParamLookup, 
            m_AudioLookup, 
            leftInputs, 
            rightInputs, 
            outputId, 
            audioCv, 
            m_MathsFunctionLookup[mathsType],
            id,
            init
        );

        m_LastCreatedProcessorModule->prepareToPlay(m_SamplesPerBlockExpected, m_SampleRate);
        m_LastCreatedProcessorModule->setReady(true);

        m_NewProcessorModuleCreated.store(true);

        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

bool MainComponent::createMathsModule(const char* json)
{
    try
    {
        auto values = json::parse(json);
        MathsModuleType mathsType = (MathsModuleType)values["type_int"].get<int>();

        std::vector<int> leftIn;
        if (values.contains("left_input_from"))
        {
            leftIn = values["left_input_from"].get<std::vector<int>>();
        }        

        int outputId = -1;
        if (values.contains("output_id"))
        {
            outputId = values["output_id"].get<int>();
            std::vector<float> temp(m_SamplesPerBlockExpected, 1);
            m_CvParamLookup.emplace(outputId, std::move(temp));
        }

        int id = values["global_index"].get<int>();

        if (mathsType == MathsModuleType::Map)
        {
            float minIn = values["min_in"].get<float>();
            float maxIn = values["max_in"].get<float>();
            float minOut = values["min_out"].get<float>();
            float maxOut = values["max_out"].get<float>();

            m_LastCreatedProcessorModule = new MathsModule(
                m_AudioLookup,
                m_CvParamLookup, 
                leftIn, 
                outputId, 
                minIn, 
                maxIn, 
                minOut, 
                maxOut, 
                mathsType,
                id
            );
            m_LastCreatedProcessorModule->prepareToPlay(m_SamplesPerBlockExpected, m_SampleRate);
            m_LastCreatedProcessorModule->setReady(true);
            m_NewProcessorModuleCreated.store(true);
        }
        else
        {
            std::vector<int> rightIn;
            if (values.contains("right_input_from"))
            {
                rightIn = values["right_input_from"].get<std::vector<int>>();
            }

            float initValue = values["initial_value"].get<float>();

            m_LastCreatedProcessorModule = new MathsModule(
                m_AudioLookup,
                m_CvParamLookup, 
                leftIn, 
                rightIn, 
                outputId, 
                mathsType, 
                m_MathsFunctionLookup[mathsType],
                id,
                initValue
            );
            m_LastCreatedProcessorModule->prepareToPlay(m_SamplesPerBlockExpected, m_SampleRate);
            m_LastCreatedProcessorModule->setReady(true);
            m_NewProcessorModuleCreated.store(true);
        }

        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

bool MainComponent::createAudioOutputModule(const char* json)
{
    try
    {
        auto values = json::parse(json);

        std::vector<int> leftIndexes, rightIndexes;

        if (values.contains("left_input_from"))
        {
            leftIndexes = values["left_input_from"].get<std::vector<int>>();
        }

        if (values.contains("right_input_from"))
        {
            rightIndexes = values["right_input_from"].get<std::vector<int>>();
        }

        int id = values["global_index"].get<int>();

        m_LastCreatedOutputModule = new AudioOutputModule(
            m_AudioLookup, 
            leftIndexes, 
            rightIndexes, 
            id
        );
        m_LastCreatedOutputModule->prepareToPlay(m_SamplesPerBlockExpected, m_SampleRate);
        m_LastCreatedOutputModule->setReady(true);
        m_NewOutputModuleCreated.store(true);

        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

bool MainComponent::createOscillatorModule(const char* json)
{
    try
    {
        auto values = json::parse(json);

        OscillatorType t = (OscillatorType)values["type_int"].get<int>();

        std::vector<int> freqIns;
        if (values.contains("freq_input_from"))
        {
            freqIns = values["freq_input_from"].get<std::vector<int>>();
        }

        std::vector<int> pwIns;
        if (values.contains("pw_input_from"))
        {
            pwIns = values["pw_input_from"].get<std::vector<int>>();
        }

        int soundIdx = -1;
        if (values.contains("sound_output_to"))
        {
            soundIdx = values["sound_output_to"].get<int>();
            juce::AudioBuffer<float> buff(2, m_SamplesPerBlockExpected);
            m_AudioLookup.emplace(soundIdx, buff);
        }

        int cvIdx = -1;
        if (values.contains("cv_out_to"))
        {
            cvIdx = values["cv_out_to"].get<int>();
            std::vector<float> temp(m_SamplesPerBlockExpected, 1);
            m_CvParamLookup.emplace(cvIdx, std::move(temp));
        }

        float frequency = 100;
        if (values.contains("initial_frequency"))
        {
            frequency = values["initial_frequency"].get<float>();
        }

        int id = values["global_index"].get<int>();

        m_LastCreatedProcessorModule = new OscillatorModule(
            t, 
            m_CvParamLookup, 
            m_AudioLookup, 
            freqIns, 
            pwIns, 
            soundIdx, 
            cvIdx,
            id,
            frequency
        );
        m_LastCreatedProcessorModule->prepareToPlay(m_SamplesPerBlockExpected, m_SampleRate);
        m_LastCreatedProcessorModule->setReady(true);
        m_NewProcessorModuleCreated.store(true);

        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

bool MainComponent::createAdsrModule(const char* json)
{
    try
    {
        auto values = json::parse(json);

        std::vector<int> inputIndexes;
        if (values.contains("input_from"))
        {
            inputIndexes = values["input_from"].get<std::vector<int>>();
        }

        int outputIndex = -1;
        if (values.contains("output_to"))
        {
            outputIndex = values["output_to"].get<int>();
            juce::AudioBuffer<float> buff(2, m_SamplesPerBlockExpected);
            m_AudioLookup.emplace(outputIndex, std::move(buff));
        }

        float attack = values["attack"].get<float>();
        float decay = values["decay"].get<float>();
        float sustain = values["sustain"].get<float>();
        float release = values["release"].get<float>();

        int id = values["global_index"].get<int>();

        m_LastCreatedProcessorModule = new ADSRModule(
            m_AudioLookup, 
            m_CvParamLookup, 
            inputIndexes,
            outputIndex,
            attack,
            decay,
            sustain,
            release,
            id
        );
        m_LastCreatedProcessorModule->prepareToPlay(m_SamplesPerBlockExpected, m_SampleRate);
        m_LastCreatedProcessorModule->setReady(true);
        m_NewProcessorModuleCreated.store(true);

        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

bool MainComponent::createNumberModule(const char* jsonText)
{
    try
    {
        auto values = json::parse(jsonText);

        std::vector<int> inputIndexes;
        if (values.contains("input_from"))
        {
            inputIndexes = values["input_from"].get<std::vector<int>>();
        }

        float initialValue = 0;
        if (values.contains("initial_value"))
        {
            initialValue = values["initial_value"].get<float>();
        }

        int outputIndex = -1;
        if (values.contains("output_to"))
        {
            outputIndex = values["output_to"].get<int>();
            std::vector<float> temp(m_SamplesPerBlockExpected, initialValue);
            m_CvParamLookup.emplace(outputIndex, std::move(temp));
        }        

        int id = values["global_index"].get<int>();

        m_LastCreatedProcessorModule = new NumberBoxModule(
            m_CvParamLookup,
            m_AudioLookup,
            inputIndexes,
            outputIndex,
            id,
            initialValue
        );

        m_LastCreatedProcessorModule->prepareToPlay(m_SamplesPerBlockExpected, m_SampleRate);
        m_LastCreatedProcessorModule->setReady(true);
        m_NewProcessorModuleCreated.store(true);

        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

bool MainComponent::createFilterModule(const char* json)
{
    try
    {
        auto j = json::parse(json);

        std::vector<int> audioInputs;
        if (j.contains("input_from"))
        {
            audioInputs = j["input_from"].get<std::vector<int>>();
        }

        std::vector<int> freqInputs;
        if (j.contains("freq_from"))
        {
            freqInputs = j["freq_from"].get<std::vector<int>>();
        }

        std::vector<int> qInputs;
        if (j.contains("q_from"))
        {
            qInputs = j["q_from"].get<std::vector<int>>();
        }

        std::vector<int> typeInputs;
        if (j.contains("type_from"))
        {
            typeInputs = j["type_from"].get<std::vector<int>>();
        }

        int id = j["global_id"].get<int>();

        int output = j["output_id"].get<int>();
        juce::AudioBuffer<float> buff(2, m_SamplesPerBlockExpected);
        m_AudioLookup.emplace(output, std::move(buff));

        if (j.contains("cutoff") && j.contains("q") && j.contains("type"))
        {
            float cutoff = j["cutoff"].get<float>();
            float q = j["q"].get<float>();
            int type = j["type"].get<int>();

            m_LastCreatedProcessorModule = new FilterModule(
                m_AudioLookup,
                m_CvParamLookup,
                audioInputs,
                freqInputs,
                qInputs,
                typeInputs,
                output,
                id,
                cutoff,
                q,
                type
            );
        }
        else
        {
            m_LastCreatedProcessorModule = new FilterModule(
                m_AudioLookup,
                m_CvParamLookup,
                audioInputs,
                freqInputs,
                qInputs,
                typeInputs,
                output,
                id
            );
        }

        m_LastCreatedProcessorModule->prepareToPlay(m_SamplesPerBlockExpected, m_SampleRate);
        m_LastCreatedProcessorModule->setReady(true);
        m_NewProcessorModuleCreated.store(true);

        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

bool MainComponent::createToggleModule(const char* json)
{
    try
    {
        auto j = json::parse(json);

        int id = j["global_index"].get<int>();        

        m_LastCreatedTriggerModule = new ToggleModule(id);
        m_LastCreatedTriggerModule->setReady(true);
        m_NewTriggerModuleCreated.store(true);

        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

bool MainComponent::createBangModule(const char* json)
{
    try
    {
        auto j = json::parse(json);

        int id = j["global_index"].get<int>();

        m_LastCreatedTriggerModule = new BangModule(id, m_AudioLookup, m_CvParamLookup);
        m_LastCreatedTriggerModule->setReady(true);
        dynamic_cast<ProcessorModule*>(m_LastCreatedTriggerModule)->prepareToPlay(m_SamplesPerBlockExpected, m_SampleRate);
        m_NewTriggerModuleCreated.store(true);

        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

bool MainComponent::createBangDelayModule(const char* jsonText)
{
    try
    {
        auto j = json::parse(jsonText);

        auto id = j["global_index"].get<int>();
        auto delay = j["delay"].get<float>();

        m_LastCreatedProcessorModule = new BangDelayModule(id, delay, m_AudioLookup, m_CvParamLookup);
        m_LastCreatedProcessorModule->setReady(true);
        m_LastCreatedProcessorModule->prepareToPlay(m_SamplesPerBlockExpected, m_SampleRate);
        m_NewProcessorModuleCreated.store(true);

        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

bool MainComponent::createSoundfileModule(const char* jsonText)
{
    try
    {
        auto j = json::parse(jsonText);

        auto id = j["global_index"].get<int>();
        auto soundOut = j["output_index"].get<int>();
        juce::AudioBuffer<float> buff(2, m_SamplesPerBlockExpected);
        m_AudioLookup.emplace(soundOut, std::move(buff));

        m_LastCreatedProcessorModule = new SoundfileModule(
            id,
            soundOut,
            m_AudioLookup,
            m_CvParamLookup
        );
        m_LastCreatedProcessorModule->setReady(true);
        m_LastCreatedProcessorModule->prepareToPlay(m_SamplesPerBlockExpected, m_SampleRate);
        m_NewProcessorModuleCreated.store(true);

        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

void MainComponent::destroyModule(int moduleType, int moduleId)
{    
    m_ModuleIdToDestroy = moduleId;
    
    switch (moduleType)
    {
        case 0:
            m_ShouldDestroyOutputModule.store(true);
            break;
        case 1:
            m_ShouldDestroyProcessorModule.store(true);
            break;
        case 2:
            m_ShouldDestroyTriggerModule.store(true);
            break;
    }
}

// ONLY call when about to deallocate the class
void MainComponent::clearModules()
{
    m_ShouldClearModules.store(true);
}
