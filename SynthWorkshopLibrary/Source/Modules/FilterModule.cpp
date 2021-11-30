/*
  ==============================================================================

    FilterModule.cpp
    Created: 24 Nov 2021 5:53:14pm
    Author:  ryand

  ==============================================================================
*/

#include "FilterModule.h"

using namespace SynthWorkshop::Modules;

FilterModule::FilterModule(AudioMap& audio, CVMap& cv, std::vector<int> audioInputs, std::vector<int> freqInputs, std::vector<int> qInputs, std::vector<int> typeInputs, int output, int id)
    : ProcessorModule(id, audio, cv), 
    m_AudioInputIndexes(audioInputs), 
    m_FreqInputIndexes(freqInputs), 
    m_QInputIndexes(qInputs), 
    m_TypeInputIndexes(typeInputs), 
    m_OutputIndex(output)
{
    m_Cutoff = 20000.f;
    m_Q = static_cast<float> (1.0 / juce::MathConstants<double>::sqrt2);
    m_FilterType = FiltType::lowPass;
}

FilterModule::FilterModule(AudioMap& audio, CVMap& cv, std::vector<int> audioInputs, std::vector<int> freqInputs, std::vector<int> qInputs, std::vector<int> typeInputs, int output, int id, float cutoff, float q, int type)
    : ProcessorModule(id, audio, cv), 
    m_AudioInputIndexes(audioInputs), 
    m_FreqInputIndexes(freqInputs), 
    m_QInputIndexes(qInputs), 
    m_TypeInputIndexes(typeInputs), 
    m_OutputIndex(output), 
    m_Cutoff(cutoff), 
    m_Q(q), 
    m_FilterType((FiltType)type)
{
}

void FilterModule::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    m_SampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlockExpected;
    spec.sampleRate = sampleRate;
    spec.numChannels = 2;
    m_Filter.prepare(spec);
    m_Filter.parameters->type = m_FilterType;
    m_Filter.parameters->setCutOffFrequency(sampleRate, m_Cutoff, m_Q);
}

void FilterModule::getNextAudioBlock(int numSamples, int numChannels)
{
    for (int sample = 0; sample < numSamples; sample++)
    {
        // get cutoff
        float cutoff = 0;
        if (m_FreqInputIndexes.size() > 0)
        {
            for (auto i : m_FreqInputIndexes)
            {
                cutoff += m_CvParamLookup[i][sample];
            }

            m_Cutoff = cutoff;
        }

        // get q
        float q = 0;
        if (m_QInputIndexes.size() > 0)
        {
            for (auto i : m_QInputIndexes)
            {
                q += m_CvParamLookup[i][sample];
            }

            m_Q = q;
        }

        // get type
        int type = 0;
        if (m_TypeInputIndexes.size() > 0)
        {
            for (auto i : m_TypeInputIndexes)
            {
                type += (int)(m_CvParamLookup[i][sample]);
            }

            if (type > 2)
            {
                type = 2;
            }

            m_FilterType = (FiltType)type;
        }

        m_Filter.parameters->type = m_FilterType;
        m_Filter.parameters->setCutOffFrequency(m_SampleRate, m_Cutoff, m_Q);

        for (int channel = 0; channel < numChannels; channel++)
        {
            float input = 0;
            for (auto i : m_AudioInputIndexes)
            {
                input += m_AudioLookup[i].getSample(channel, sample);
            }

            m_AudioLookup[m_OutputIndex].setSample(channel, sample, m_Filter.processSample(input));
        }
    }
}

void FilterModule::releaseResources()
{
    m_Filter.reset();
}

void FilterModule::setInputIndex(int outputIndex, int targetIndex)
{
    juce::ScopedLock sl(m_Cs);

    switch (targetIndex)
    {
        case 0:
            m_AudioInputIndexes.push_back(outputIndex);
            break;
        case 1:
            m_FreqInputIndexes.push_back(outputIndex);
            break;
        case 2:
            m_QInputIndexes.push_back(outputIndex);
            break;
        case 3:
            m_TypeInputIndexes.push_back(outputIndex);
            break;
    }
}

void FilterModule::removeInputIndex(int outputIndex, int targetIndex)
{
    juce::ScopedLock sl(m_Cs);

    std::vector<int>* vec = nullptr;

    switch (targetIndex)
    {
        case 0:
            vec = &m_AudioInputIndexes;
            break;
        case 1:
            vec = &m_FreqInputIndexes;
            break;
        case 2:
            vec = &m_QInputIndexes;
            break;
        case 3:
            vec = &m_TypeInputIndexes;
            break;
    }

    if (vec == nullptr) return;

    auto it = std::find(vec->begin(), vec->end(), outputIndex);
    if (it != vec->end())
    {
        vec->erase(it);
    }
}
