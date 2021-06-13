/*
  ==============================================================================

    Exports.h
    Created: 12 Jun 2021 7:00:55pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include "MainComponent.h"

#define EXPORT _declspec(dllexport)

extern "C" {

    EXPORT const char* helloWorld() {
        return "Hello World!";
    }

    EXPORT void* initialise() {
        juce::initialiseJuce_GUI();
        auto ptr = new MainComponent();
        return (void*)ptr;
    }

    EXPORT void shutdown(void* mc) {
        delete (MainComponent*)mc;
        juce::shutdownJuce_GUI();
    }

    EXPORT const char* helloWorldFromMain(void* mc) {
        return ((MainComponent*)mc)->helloWorld();
    }

    EXPORT bool createModulesFromJson(void* mc, const char* jsonText) {
        return ((MainComponent*)mc)->createModulesFromJson(jsonText);
    }

    EXPORT void setCvParam(void* mc, int idx, float val) {
        ((MainComponent*)mc)->setCvParam(idx, val);
    }
}