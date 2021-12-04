/*
  ==============================================================================

    ADSRModule.cpp
    Created: 24 Nov 2021 5:40:31pm
    Author:  ryand

  ==============================================================================
*/

#include "ADSRModule.h"

using namespace SynthWorkshop::Modules;

ADSRModule::ADSRModule(AudioMap& audioLu, CVMap& cvLookup, std::vector<int> inputIndexes, int outputIndex, float attack, float decay, float sustain, float release, int id)
    : ProcessorModule(id, audioLu, cvLookup),
    Triggerable(id),
    m_AudioInputIndexes(inputIndexes),
    m_AudioOutputIndex(outputIndex),
    m_Attack(attack),
    m_Decay(decay),
    m_Sustain(sustain),
    m_Release(release)
{
}

void ADSRModule::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    m_SampleRate = sampleRate;
    m_Envelope.setSampleRate(m_SampleRate);
}

void ADSRModule::getNextAudioBlock(int numSamples, int numChannels)
{
    if (!m_ReadyToPlay || m_AudioOutputIndex == -1) return;

    m_AudioLookup[m_AudioOutputIndex].clear();
    auto write = m_AudioLookup[m_AudioOutputIndex].getArrayOfWritePointers();

    for (int sample = 0; sample < numSamples; sample++)
    {
        updateEnvelope(sample);

        float value = m_Envelope.getNextSample();

        for (int channel = 0; channel < numChannels; channel++)
        {
            for (int i : m_AudioInputIndexes)
            {
                write[channel][sample] += (m_AudioLookup[i].getSample(channel, sample) * value);
            }
        }
    }
}

void ADSRModule::setInputIndex(int outputIndex, int targetIndex)
{
    juce::ScopedLock sl(m_Cs);

    switch (targetIndex)
    {
        case 0:
            m_AudioInputIndexes.push_back(outputIndex);
            break;
        case 1:
            m_TriggerInputs.push_back(outputIndex);
            break;
        case 2:
            m_AttackIndexes.push_back(outputIndex);
            break;
        case 3:
            m_DecayIndexes.push_back(outputIndex);
            break;
        case 4:
            m_SustainIndexes.push_back(outputIndex);
            break;
        case 5:
            m_ReleaseIndexes.push_back(outputIndex);
            break;
    }
}

void ADSRModule::removeInputIndex(int outputIndex, int targetIndex)
{
    juce::ScopedLock sl(m_Cs);

    std::vector<int>* vec = nullptr;
    switch (targetIndex)
    {
        case 0:
            vec = &m_AudioInputIndexes;
            break;
        case 1:
            vec = &m_TriggerInputs;
            break;
        case 2:
            vec = &m_AttackIndexes;
            break;
        case 3:
            vec = &m_DecayIndexes;
            break;
        case 4:
            vec = &m_SustainIndexes;
            break;
        case 5:
            vec = &m_ReleaseIndexes;
            break;
    }

    if (vec == nullptr) return;

    auto it = std::find(vec->begin(), vec->end(), outputIndex);
    if (it != vec->end())
    {
        vec->erase(it);
    }
}

void ADSRModule::triggerCallback(Triggerable* sender, bool state)
{
    if (state)
    {
        m_EnvelopeShouldNoteOn.store(true);
    }
    else
    {
        m_EnvelopeShouldNoteOff.store(true);
    }
}

void ADSRModule::updateEnvelope(int sample)
{
    if (!m_AttackIndexes.empty())
    {
        float attack = 0;
        for (auto i : m_AttackIndexes)
        {            
            attack += m_CvParamLookup[i][sample];
        }
        m_Attack = attack;
    }

    if (!m_DecayIndexes.empty())
    {
        float decay = 0;
        for (auto i : m_DecayIndexes)
        {
            decay += m_CvParamLookup[i][sample];
        }
        m_Decay = decay;
    }

    if (!m_SustainIndexes.empty())
    {
        float sustain = 0;
        for (auto i : m_SustainIndexes)
        {
            sustain += m_CvParamLookup[i][sample];
        }
        m_Sustain = sustain;
    }

    if (!m_ReleaseIndexes.empty())
    {
        float release = 0;
        for (auto i : m_ReleaseIndexes)
        {
            release += m_CvParamLookup[i][sample];
        }
        m_Release = release;
    }

    juce::ADSR::Parameters params(m_Attack, m_Decay, m_Sustain, m_Release);
    m_Envelope.setParameters(params);

    if (m_EnvelopeShouldNoteOn.load())
    {
        m_Envelope.noteOn();
        m_EnvelopeShouldNoteOn.store(false);
    }

    if (m_EnvelopeShouldNoteOff.load())
    {
        m_Envelope.noteOff();
        m_EnvelopeShouldNoteOff.store(false);
    }
}
