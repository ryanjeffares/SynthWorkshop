/*
  ==============================================================================

    OscillatorModule.cpp
    Created: 24 Nov 2021 6:05:21pm
    Author:  ryand

  ==============================================================================
*/

#include "OscillatorModule.h"

using namespace SynthWorkshop::Modules;
using namespace SynthWorkshop::Enums;

OscillatorModule::OscillatorModule(OscillatorType t, CVMap& cvLookup, AudioMap& audioLu, std::vector<int> freqIdx, std::vector<int> pwIdx, int soundIdx, int cvOutIdx, int id, float freq)
    : ProcessorModule(id, audioLu, cvLookup),
    Triggerable(id),
    m_OscillatorType(t), 
    m_StartingFreq(freq), 
    m_Phase(0.f), 
    m_FrequencyInputIndexes(freqIdx), 
    m_PulsewidthInputIndexes(pwIdx),
    m_SoundOutIndex(soundIdx), 
    m_CvOutIndex(cvOutIdx),
    m_ShouldResetPhase(false)
{
}

void OscillatorModule::prepareToPlay(int samplesPerBlock, double sr)
{
    m_SampleRate = sr;
}

void OscillatorModule::getNextAudioBlock(int numSamples, int numChannels)
{
    if (!m_ReadyToPlay) return;

    for (auto sample = 0; sample < numSamples; sample++)
    {
        float frequency = 0;
        if (m_FrequencyInputIndexes.size() > 0)
        {
            for (auto i : m_FrequencyInputIndexes)
            {
                frequency += m_CvParamLookup[i][sample];
            }

            m_StartingFreq = frequency;
        }
        else
        {
            frequency = m_StartingFreq;
        }

        float pulseWidth = 0.5f;
        if (m_PulsewidthInputIndexes.size() > 0)
        {
            pulseWidth = 0;
            for (auto i : m_PulsewidthInputIndexes)
            {
                pulseWidth += m_CvParamLookup[i][sample];
            }
        }

        float value = getNextSample(frequency, pulseWidth);

        if (m_SoundOutIndex != -1)
        {
            for (auto channel = 0; channel < numChannels; channel++)
            {
                m_AudioLookup[m_SoundOutIndex].setSample(channel, sample, value);
            }
        }

        if (m_CvOutIndex != -1)
        {
            m_CvParamLookup[m_CvOutIndex][sample] = value;
        }
    }
}

void OscillatorModule::setInputIndex(int outputIndex, int targetIndex)
{
    juce::ScopedLock sl(m_Cs);

    switch (targetIndex)
    {
        case 2:
            m_FrequencyInputIndexes.push_back(outputIndex);
            break;
        case 3:
            m_PulsewidthInputIndexes.push_back(outputIndex);
            break;
    }
}

void OscillatorModule::removeInputIndex(int outputIndex, int targetIndex)
{
    juce::ScopedLock sl(m_Cs);

    std::vector<int>* vec = nullptr;
    switch (targetIndex)
    {
        case 2:
            vec = &m_FrequencyInputIndexes;
            break;
        case 3:
            vec = &m_PulsewidthInputIndexes;
            break;
    }

    if (vec == nullptr) return;

    auto it = std::find(vec->begin(), vec->end(), outputIndex);
    if (it != vec->end())
    {
        vec->erase(it);
    }
}

void OscillatorModule::triggerCallback(Triggerable* sender, bool state)
{
    m_ShouldResetPhase.store(state);
}

float OscillatorModule::getNextSample(float frequency, float pulseWidth)
{
    if (m_ShouldResetPhase.load())
    {
        m_Phase = 0;
        m_ShouldResetPhase.store(false);
    }

    switch (m_OscillatorType)
    {
        case OscillatorType::Saw:
            m_Output = m_Phase;
            if (m_Phase >= 1.f)
            {
                m_Phase -= 2.f;
            }
            m_Phase += (1.f / (m_SampleRate / frequency)) * 2.f;
            return m_Output;
        case OscillatorType::Pulse:
            if (m_Phase >= 1.f)
            {
                m_Phase -= 1.f;
            }
            m_Phase += (1.f / (m_SampleRate / frequency));
            if (m_Phase < pulseWidth)
            {
                m_Output = -1;
            }
            if (m_Phase > pulseWidth)
            {
                m_Output = 1;
            }
            return m_Output;
        case OscillatorType::Sine:
            m_Output = std::sin(m_Phase * juce::MathConstants<float>::twoPi);
            if (m_Phase >= 1.f)
            {
                m_Phase -= 1.f;
            }
            m_Phase += (1.f / (m_SampleRate / frequency));
            return m_Output;
        case OscillatorType::Tri:
            if (m_Phase >= 1.f)
            {
                m_Phase -= 1.f;
            }
            m_Phase += (1.f / (m_SampleRate / frequency));
            if (m_Phase <= 0.5f)
            {
                m_Output = (m_Phase - 0.25f) * 4;
            }
            else
            {
                m_Output = ((1.f - m_Phase) - 0.25f) * 4;
            }
            return m_Output;
        default: return 0;
    }
}
