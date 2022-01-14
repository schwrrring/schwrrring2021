//
// Created by Malte Hildebrand on 21.09.21.
/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 hello_world_module
  vendor:             malte_Hildebrand
  version:            0.0.1
  name:               Easy Ears Hello World Class
  description:        It is the beginning of my tested Project
  website:            http://www.juce.com/juce
  license:            GPL/Commercial

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/

namespace schwrrring
{

}
#pragma once
#include "Utilities/schwrrring_identifiers.h"
#define HELLO_WORLD_MODULE_H_INCLUDED
//#include "../../Source/PracticeTimes.h"
#include "common/Utilities.h"
#include "common/Components.h"
#include "Components/ChordSheetComponent.h"
#include "common/ChordClipMapper.h"
#include "Components/ChordComponent.h"
#include "HeaderComponent.h"
#include "PracticeComponent.h"
#include "MidiInputList.h"
#include "PracticeTimes.h"
#include "common/PluginWindow.h"

namespace hello_world_module
{
    class ExtendedUIBehaviour;
}
#define UNITTESTRUNNER_HELLOWORLDMODULE_H

/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/
#if EASY_EARS_UNIT_TESTS
namespace juce {
    namespace dsp {

        struct LinearAlgebraUnityTest : public UnitTest {
            LinearAlgebraUnityTest()
                    : UnitTest("Linear Algebra Malte", UnitTestCategories::easyears) {}

            struct AdditionTest {
                template<typename ElementType>
                static void run(LinearAlgebraUnityTest &u) {
//                    juce: alo = 1;ß
                    juce::Reverb reverb;
                    u.expect((1 + 2) == 3);
//                    auto    practiceTimes = new PracticeTimes();

                }
            };



            template<class TheTest>
            void runTestForAllTypes(const char *unitTestName) {
                beginTest(unitTestName);

                TheTest::template run<float>(*this);
                TheTest::template run<double>(*this);
            }

            void runTest() override {
                runTestForAllTypes<AdditionTest>("AdditionTest");

            }
        };

        static LinearAlgebraUnityTest linearAlgebraUnitTest;

    } // namespace dsp
} // namespace juce
#endif
namespace juce {
    static String helloWorld() {
        return "hello world";
    }
}


