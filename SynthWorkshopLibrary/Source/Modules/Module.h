/*
  ==============================================================================

    Module.h
    Created: 15 Oct 2021 10:47:59pm
    Author:  Ryan Jeffares

  ==============================================================================
*/

#pragma once

class Module
{
public:
    
    Module() = default;
    virtual ~Module() = default;
    
    virtual void prepareToPlay(int samplesPerBlockExpected, double sampleRate) {}
    virtual void getNextAudioBlock(int numSamples, int numChannels) {}
    virtual void releaseResources() {}
    
    void setReady(bool state) {
        readyToPlay = state;
    }
    
protected:
    bool readyToPlay = false;
};
