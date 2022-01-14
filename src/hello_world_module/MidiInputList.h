//
// Created by Malte Hildebrand on 06.09.21.
//

#ifndef GUI_APP_EXAMPLE_2_MIDIINPUTLIST_H
#define GUI_APP_EXAMPLE_2_MIDIINPUTLIST_H

#pragma once

#include <JuceHeader.h>

// CMake builds don't use an AppConfig.h, so it's safe to include juce module headers
// directly. If you need to remain compatible with Projucer-generated builds, and
// have called `juce_generate_juce_header(<thisTarget>)` in your CMakeLists.txt,
// you could `#include <JuceHeader.h>` here instead, to make all your module headers visible.
#include <juce_gui_extra/juce_gui_extra.h>
#include "common/Utilities.h"


class  MidiInputList : public juce::ComboBox{
public:
    MidiInputList(){
        midiInputListLabel.setText("MIDI Input:", juce::dontSendNotification);
        midiInputListLabel.attachToComponent(&midiInputList, true);

        midiInputList.setTextWhenNoChoicesAvailable("No MIDI Inputs Enabled");
        auto midiInputs = juce::MidiInput::getAvailableDevices();

        juce::StringArray midiInputNames;

        for (auto input : midiInputs)
            midiInputNames.add(input.name);

        midiInputList.addItemList(midiInputNames, 1);
        midiInputList.onChange = [this] { setMidiInput(midiInputList.getSelectedItemIndex()); };

        // find the first enabled device and use that by default
        for (auto input : midiInputs) {
            if (deviceManager.isMidiInputDeviceEnabled(input.identifier)) {
                setMidiInput(midiInputs.indexOf(input));
                break;
            }
        }

        // if no enabled devices were found just use the first one in the list
        if (midiInputList.getSelectedId() == 0)
            setMidiInput(0);

        midiMessagesBox.setMultiLine(true);
        midiMessagesBox.setReturnKeyStartsNewLine(true);
        midiMessagesBox.setReadOnly(true);
        midiMessagesBox.setScrollbarsShown(true);
        midiMessagesBox.setCaretVisible(false);
        midiMessagesBox.setPopupMenuEnabled(true);
        midiMessagesBox.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x32ffffff));
        midiMessagesBox.setColour(juce::TextEditor::outlineColourId, juce::Colour(0x1c000000));
        midiMessagesBox.setColour(juce::TextEditor::shadowColourId, juce::Colour(0x16000000));

    }
    void paint(juce::Graphics &g) override {
        //                einfach nen rectangle über die ganze weite called Header
        g.fillCheckerBoard(getLocalBounds().toFloat(), 30, 10,
                           juce::Colours::sandybrown, juce::Colours::saddlebrown);

    }



    void resized() override {

    }

private:
    int lastInputIndex = 0;                           // [3]
    juce::ComboBox midiInputList;                     // [2]
    juce::Label midiInputListLabel;
    Label editNameLabel{"No Edit Loaded"};
    //    ToggleButton showWaveformButton{"Show Waveforms"};
    juce::AudioDeviceManager deviceManager;           // [1]
    juce::TextEditor midiMessagesBox;


    /** Starts listening to a MIDI input device, enabling it if necessary. */
    void setMidiInput(int index) {
        auto list = juce::MidiInput::getAvailableDevices();
        //todo: this mit MidiRecording verbinden
        deviceManager.removeMidiInputDeviceCallback(list[lastInputIndex].identifier, dynamic_cast<juce::MidiInputCallback*>( this->getParentComponent()));

        auto newInput = list[index];

        if (!deviceManager.isMidiInputDeviceEnabled(newInput.identifier))
            deviceManager.setMidiInputDeviceEnabled(newInput.identifier, true);
//        todo: statt this MidiRecording
deviceManager.addMidiInputDeviceCallback(newInput.identifier, dynamic_cast<juce::MidiInputCallback*>( this->getParentComponent()));
        midiInputList.setSelectedId(index + 1, juce::dontSendNotification);

        lastInputIndex = index;
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiInputList)
};
#endif //GUI_APP_EXAMPLE_2_MIDIINPUTLIST_H
