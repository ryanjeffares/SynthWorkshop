/*
  ==============================================================================

    Module.h
    Created: 15 Oct 2021 10:47:59pm
    Author:  Ryan Jeffares

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class Module
{
public:
    
    Module() = default;
    virtual ~Module() = default;

    using AudioMap = std::unordered_map<int, juce::AudioBuffer<float>>;
    using CVMap = std::unordered_map<int, std::vector<float>>;
    
    virtual void prepareToPlay(int samplesPerBlockExpected, double sampleRate) {}
    virtual void getNextAudioBlock(int numSamples, int numChannels) {}
    virtual void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) {}
    virtual void releaseResources() {}
    
    virtual void setReady(bool) = 0;
    virtual void setInputIndex(int outputIndex, int targetIndex) = 0;
    virtual void removeInputIndex(int outputIndex, int targetIndex) = 0;
    
    int getModuleId() const { return m_ModuleId; }

protected:

    bool m_ReadyToPlay = false;
    int m_ModuleId;

    juce::CriticalSection m_Cs;
};
