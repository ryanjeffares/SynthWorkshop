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

    MathsModule(std::unordered_map<int, float>& lookup, int leftIn, int rightIn, int output, int inMin, int inMax, int outMin, int outMax, MathsModuleType t) : cvParamLookup(lookup),
        leftInput(leftIn), rightInput(rightIn), outputIndex(output), minIn(inMin), minOut(outMin), maxIn(inMax), maxOut(outMax), type(t) {}

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
            case MathsModuleType::Mod:
                cvParamLookup[outputIndex] = fmod(left, right);
                break;
            case MathsModuleType::Sin:
                cvParamLookup[outputIndex] = sin(left);
                break;
            case MathsModuleType::Cos:
                cvParamLookup[outputIndex] = cos(left);
                break;
            case MathsModuleType::Tan:
                cvParamLookup[outputIndex] = tan(left);
                break;
            case MathsModuleType::Asin:
                cvParamLookup[outputIndex] = asin(left);
                break;
            case MathsModuleType::Acos:
                cvParamLookup[outputIndex] = acos(left);
                break;
            case MathsModuleType::Atan:
                cvParamLookup[outputIndex] = atan(left);
                break;
            case MathsModuleType::Abs:
                cvParamLookup[outputIndex] = fabs(left);
                break;
            case MathsModuleType::Exp:
                cvParamLookup[outputIndex] = pow(left, right);
                break;
            case MathsModuleType::Int:
                cvParamLookup[outputIndex] = (int)left;
                break;
            case MathsModuleType::Map:
                cvParamLookup[outputIndex] = juce::jmap<float>(left, cvParamLookup[minIn], cvParamLookup[maxIn], cvParamLookup[minOut], cvParamLookup[maxOut]);
                break;
            case MathsModuleType::Mtof:
                cvParamLookup[outputIndex] = swMtof((int)cvParamLookup[leftInput]);
                break;
            case MathsModuleType::Ftom:
                cvParamLookup[outputIndex] = swFtom(cvParamLookup[leftInput]);
                break;
        }
    }

private:

    std::unordered_map<int, float>& cvParamLookup;
    const int leftInput, rightInput, outputIndex;
    const MathsModuleType type;

    // these are indexes for the lookup
    int minIn, maxIn, minOut, maxOut;

};