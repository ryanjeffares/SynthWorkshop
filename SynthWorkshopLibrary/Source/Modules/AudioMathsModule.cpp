/*
  ==============================================================================

    AudioMathsModule.cpp
    Created: 24 Nov 2021 5:44:00pm
    Author:  ryand

  ==============================================================================
*/

#include "AudioMathsModule.h"

using namespace SynthWorkshop::Modules;
using namespace SynthWorkshop::Enums;

AudioMathsModule::AudioMathsModule(CVMap& cvLookup, AudioMap& audioLu, std::vector<int> leftIn, std::vector<int> rightIns, int output, AudioCV acv, const std::function<float(float, float)>& func, int id, float init)
    : ProcessorModule(id, audioLu, cvLookup),
    m_LeftInputs(leftIn),
    m_RightInputs(rightIns),
    m_OutputIndex(output),
    m_AudioCvIncoming(acv),
    m_CurrentFunction(func),
    m_InitialValue(init)
{
}

void AudioMathsModule::prepareToPlay(int spbe, double sr)
{
    m_SampleRate = sr;
}

void AudioMathsModule::getNextAudioBlock(int numSamples, int numChannels)
{
    if (!m_ReadyToPlay || m_OutputIndex == -1) return;
    auto write = m_AudioLookup[m_OutputIndex].getArrayOfWritePointers();
    for (int sample = 0; sample < numSamples; sample++)
    {
        for (int channel = 0; channel < numChannels; channel++)
        {
            float affectingVal = 0.f;

            if (m_RightInputs.size() > 0)
            {
                for (auto r : m_RightInputs)
                {
                    if (m_AudioCvIncoming == AudioCV::Audio)
                    {
                        if (mapContains(m_AudioLookup, r))
                        {
                            affectingVal += m_AudioLookup[r].getSample(channel, sample);
                        }
                    }
                    else
                    {
                        if (mapContains(m_AudioLookup, r))
                        {
                            affectingVal += m_CvParamLookup[r][sample];
                        }
                    }
                }

                m_InitialValue = affectingVal;
            }
            else
            {
                affectingVal = m_InitialValue;
            }

            float inputSampleVal = 0.f;
            for (auto l : m_LeftInputs)
            {
                inputSampleVal += m_AudioLookup[l].getSample(channel, sample);
            }

            write[channel][sample] = m_CurrentFunction(inputSampleVal, affectingVal);
        }
    }
}

void AudioMathsModule::setInputIndex(int outputIndex, int targetIndex)
{
    juce::ScopedLock sl(m_Cs);

    switch (targetIndex)
    {
        case 0:
            m_LeftInputs.push_back(outputIndex);
            break;
        case 1:
            m_RightInputs.push_back(outputIndex);
            break;
    }
}

void AudioMathsModule::removeInputIndex(int outputIndex, int targetIndex)
{
    juce::ScopedLock sl(m_Cs);

    std::vector<int>* vec = nullptr;
    switch (targetIndex)
    {
        case 0:
            vec = &m_LeftInputs;
            break;
        case 1:
            vec = &m_RightInputs;
            break;
    }

    if (vec == nullptr) return;

    auto it = std::find(vec->begin(), vec->end(), outputIndex);
    if (it != vec->end())
    {
        vec->erase(it);
    }
}

void AudioMathsModule::setIncomingSignalType(AudioCV type)
{
    juce::ScopedLock sl(m_Cs);
    m_AudioCvIncoming = type;
}
