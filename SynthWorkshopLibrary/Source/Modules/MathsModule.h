/*
  ==============================================================================

    MathsModule.h
    Created: 20 Jun 2021 10:45:35pm
    Author:  ryand

  ==============================================================================
*/

#pragma once
#include <unordered_map>
#include "../Typedefs.h"

class MathsModule
{
public:    

    MathsModule(std::unordered_map<int, float>& lookup, int leftIn, int rightIn, int output, MathsModuleType t) : cvParamLookup(lookup), 
        leftInput(leftIn), rightInput(rightIn), outputIndex(output), type(t) {}

    ~MathsModule() {}

    void calculateValues() {
        if (outputIndex == -1) return;
        float left = leftInput == -1 ? 1 : cvParamLookup[leftInput];
        float right = rightInput == -1 ? 1 : cvParamLookup[rightInput];
        switch (type) {
            case MathsModuleType::Plus:
                cvParamLookup[outputIndex] = left + right;
                break;
            case MathsModuleType::Minus:
                cvParamLookup[outputIndex] = left - right;
                break;
            case MathsModuleType::Multiply:
                cvParamLookup[outputIndex] = left * right;
                break;
            case MathsModuleType::Divide:
                cvParamLookup[outputIndex] = left / right;
                break;
        }
    }

private:

    std::unordered_map<int, float>& cvParamLookup;
    const int leftInput, rightInput, outputIndex;
    const MathsModuleType type;

};