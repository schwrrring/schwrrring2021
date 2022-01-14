//
// Created by Malte Hildebrand on 16.09.21.
//

#ifndef EASYEARS_PRACTICETIMES_H
#define EASYEARS_PRACTICETIMES_H

#pragma once

#include <JuceHeader.h>

// CMake builds don't use an AppConfig.h, so it's safe to include juce module headers
// directly. If you need to remain compatible with Projucer-generated builds, and
// have called `juce_generate_juce_header(<thisTarget>)` in your CMakeLists.txt,
// you could `#include <JuceHeader.h>` here instead, to make all your module headers visible.
#include <juce_gui_extra/juce_gui_extra.h>
#include "common/Utilities.h"
using namespace juce;
struct PracticeTimes {

    public:
        static void  setNewPracticeTime(tracktion_engine::Track & track){

    }
    PracticeTimes() = default;

    PracticeTimes(const String &s) {
          tokens = StringArray::fromTokens(s, "|", "");
          for(juce::String(token) : tokens){
             times.add(std::stol(token.toStdString()));
         }
    }

    String toString() const {
        StringArray s;
        for(auto time : times){
            s.add( (juce::String) time);
        }
        return s.joinIntoString("|");
    }
    StringArray tokens;
    Array<long> times;
};

#endif //EASYEARS_PRACTICETIMES_H
