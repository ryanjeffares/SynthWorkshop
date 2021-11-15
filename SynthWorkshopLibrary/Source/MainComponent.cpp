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
    : m_ModulesCreated(true), m_ShouldStopAudio(false), m_ModuleCreationCanProceed(false), m_FirstRun(true), m_NewProcessorModuleCreated(false), m_NewOutputModuleCreated(false), m_ShouldDestroyOutputModule(false), m_ShouldDestroyProcessorModule(false), m_MasterVolume(0.1f)
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
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) 
{    
    for (auto& buff : m_AudioLookup)
    {
        buff.second.clear();
    }

    checkForUpdates();

    if (!m_ModulesCreated.load())
    {
        if (!m_FirstRun.load() && !m_UnityThreadNotified) 
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_ModuleCreationCanProceed.store(true);
            m_WaitCondition.notify_all();
            m_UnityThreadNotified = true;
        }
        return;
    }

    if (m_ShouldStopAudio.load()) return;
    
    int numSamples = bufferToFill.buffer->getNumSamples();
    int numChannels = bufferToFill.buffer->getNumChannels();
    
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
        m_AudioOutputModules.clear();
        m_ProcessorModules.clear();

        m_ShouldDestroyOutputModule.store(false);
        m_ShouldDestroyProcessorModule.store(false);
        m_NewOutputModuleCreated.store(false);
        m_NewProcessorModuleCreated.store(false);

        return;
    }

    // erase the any desired modules
    if (m_ShouldDestroyOutputModule.load())
    {
        auto& it = std::find_if(
            m_AudioOutputModules.begin(),
            m_AudioOutputModules.end(),
            [&](const std::unique_ptr<Module>& mod)
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

    if (m_ShouldDestroyProcessorModule.load())
    {
        auto it = std::find_if(
            m_ProcessorModules.begin(),
            m_ProcessorModules.end(),
            [&](const std::unique_ptr<Module>& mod)
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
        std::unique_ptr<Module> p(m_LastCreatedProcessorModule);
        m_ProcessorModules.push_back(std::move(p));
        m_LastCreatedProcessorModule = nullptr;
        m_NewProcessorModuleCreated.store(false);
    }

    if (m_NewOutputModuleCreated.load())
    {
        std::unique_ptr<Module> p(m_LastCreatedOutputModule);
        m_AudioOutputModules.push_back(std::move(p));
        m_LastCreatedProcessorModule = nullptr;
        m_NewOutputModuleCreated.store(false);
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
    else 
    {
        return 0;
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
    auto& vec = audioModule ? m_AudioOutputModules : m_ProcessorModules;
    for (auto& mod : vec)
    {
        if (mod->getModuleId() == moduleId)
        {
            if (add)
            {
                mod->setInputIndex(outputIndex, targetIndex);
            }
            else 
            {
                mod->removeInputIndex(outputIndex, targetIndex);
            }

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
        [moduleId](const std::unique_ptr<Module>& mod)
        {
            return mod->getModuleId() == moduleId;
        }
    );

    if (it != m_ProcessorModules.end())
    {
        auto mathsModule = dynamic_cast<AudioMathsModule*>(it->get());
        if (mathsModule != nullptr)
        {
            mathsModule->setIncomingSignalType((AudioCV)type);
        }
    }
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

        int triggerInput = -1;
        if (values.contains("trigger_input"))
        {
            triggerInput = values["trigger_input"].get<int>();
        }

        int outputIndex = -1;
        if (values.contains("output_to"))
        {
            outputIndex = values["output_to"].get<int>();
            juce::AudioBuffer<float> buff(2, m_SamplesPerBlockExpected);
            m_AudioLookup.emplace(outputIndex, std::move(buff));
        }

        // for now, these are required to exist
        int attack = values["attack_from"].get<int>();
        int decay = values["decay_from"].get<int>();
        int sustain = values["sustain_from"].get<int>();
        int release = values["release_from"].get<int>();

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
            triggerInput,
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

void MainComponent::destroyModule(bool audio, int moduleId)
{    
    m_ModuleIdToDestroy = moduleId;
    if (audio)
    {
        m_ShouldDestroyOutputModule.store(true);
    }
    else
    {
        m_ShouldDestroyProcessorModule.store(true);
    }
}

// ONLY call when about to deallocate the class
void MainComponent::clearModules()
{
    m_ShouldClearModules.store(true);
}

bool MainComponent::createModulesFromJson(const char* jsonText) 
{        
    stopAudio();
    m_ModulesCreated.store(false);
    
    try 
    {           
        if (!m_FirstRun.load())
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            while (!m_ModuleCreationCanProceed.load())
            {
                m_WaitCondition.wait(lock);
            }
        }
        
        m_ProcessorModules.clear();
        m_AudioOutputModules.clear();
        m_CvParamLookup.clear();
        m_AudioLookup.clear();

        auto j = json::parse(jsonText);
        
        for (const auto& moduleType : j.items()) 
        {    
            const auto& modType = moduleType.key();
            for (const auto& mod : moduleType.value().items()) 
            {
                if (modType == "oscillator_modules") 
                {
                    createOscillatorModuleFromJson(mod.value());
                }
                else if (modType == "io_modules") 
                {
                    createAudioOutputModuleFromJson(mod.value());
                }
                else if (modType == "maths_modules") 
                {
                    const auto& values = mod.value();
                    const auto& type = values["type_string"].get<std::string>();
                    if (type == "CV") 
                    {
                        createMathsModuleFromJson(values);
                    }
                    else if (type == "Audio") 
                    {
                        createAudioMathsModuleFromJson(values);
                    }
                }
                else if (modType == "adsr_modules") 
                {
                    createAdsrModuleFromJson(mod.value());
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
                            if (m_CvParamLookup.find(idx) == m_CvParamLookup.end()) 
                            {
                                std::vector<float> temp(m_SamplesPerBlockExpected, initial);
                                m_CvParamLookup.emplace(idx, std::move(temp));
                            }
                            else 
                            {
                                std::fill(m_CvParamLookup[idx].begin(), m_CvParamLookup[idx].end(), initial);
                            }
                        }
                    }                    
                }
            }
        }
         
        for (auto& io : m_AudioOutputModules) 
        {
            io->prepareToPlay(m_SamplesPerBlockExpected, m_SampleRate);
            io->setReady(true);
        }
        
        for (auto& module : m_ProcessorModules) 
        {
            module->prepareToPlay(m_SamplesPerBlockExpected, m_SampleRate);
            module->setReady(true);
        }

        m_ModulesCreated.store(true);
        m_ShouldStopAudio.store(false);
        m_ModuleCreationCanProceed.store(false);
        m_FirstRun.store(false);
        m_UnityThreadNotified = false;
        
        return true;
    }
    catch (const std::exception& e) 
    {        
        return false;
    }    
}

void MainComponent::createAudioMathsModuleFromJson(const json& values) 
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
        juce::AudioBuffer<float> buff(2, m_SamplesPerBlockExpected);
        m_AudioLookup.emplace(outputId, buff);
    }
    
    int id = values["global_index"].get<int>();
    float init = values["initial_value"].get<float>();

    auto mathsModule = std::make_unique<AudioMathsModule>(
        m_CvParamLookup, m_AudioLookup, leftInputs, rightInputs, outputId, audioCv, m_MathsFunctionLookup[mathsType], id, init
    );
    m_ProcessorModules.push_back(std::move(mathsModule));
}

void MainComponent::createMathsModuleFromJson(const json& values) 
{
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
        int minIn = values["min_in"].get<int>();
        int maxIn = values["max_in"].get<int>();
        int minOut = values["min_out"].get<int>();
        int maxOut = values["max_out"].get<int>();

        auto mathsModule = std::make_unique<MathsModule>(
            m_CvParamLookup, leftIn, outputId, minIn, maxIn, minOut, maxOut, mathsType, id
        );
        m_ProcessorModules.push_back(std::move(mathsModule));
    }
    else 
    {
        float init = values["initial_value"].get<float>();

        std::vector<int> rightIn;
        if (values.contains("right_input_from"))
        {
            rightIn = values["right_input_from"].get<std::vector<int>>();
        }

        auto mathsModule = std::make_unique<MathsModule>(
            m_CvParamLookup, leftIn, rightIn, outputId, mathsType, m_MathsFunctionLookup[mathsType], id, init
        );
        m_ProcessorModules.push_back(std::move(mathsModule));
    }
}

void MainComponent::createAudioOutputModuleFromJson(const json& values) 
{
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

    auto audioOut = std::make_unique<AudioOutputModule>(
        m_AudioLookup, leftIndexes, rightIndexes, id
    );
    m_AudioOutputModules.push_back(std::move(audioOut));
}

void MainComponent::createOscillatorModuleFromJson(const json& values) 
{    
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

    int id = values["global_index"].get<int>();

    auto osc = std::make_unique<OscillatorModule>(
        t, m_CvParamLookup, m_AudioLookup, freqIns, pwIns, soundIdx, cvIdx, id
    );
    m_ProcessorModules.push_back(std::move(osc));
}

void MainComponent::createAdsrModuleFromJson(const json& values) 
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
        juce::AudioBuffer<float> buff(2, m_SamplesPerBlockExpected);
        m_AudioLookup.emplace(outputIndex, buff);
    }

    // for now, these are required to exist
    int attack = values["attack_from"].get<int>();
    int decay = values["decay_from"].get<int>();
    int sustain = values["sustain_from"].get<int>();
    int release = values["release_from"].get<int>();
    int id = values["global_index"].get<int>();

    auto adsr = std::make_unique<ADSRModule>(
        m_AudioLookup, m_CvParamLookup, inputIndexes, outputIndex, attack, decay, sustain, release, triggerInput, id
    );
    m_ProcessorModules.push_back(std::move(adsr));
}
