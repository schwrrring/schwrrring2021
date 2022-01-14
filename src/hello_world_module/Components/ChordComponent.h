//
// Created by Malte Hildebrand on 27.09.21.
//


#pragma once

#include <JuceHeader.h>

//==============================================================================
//==============================================================================
class ChordComponent : public juce::Component,
        private juce::ValueTree::Listener {
public:
    ChordComponent(juce::ValueTree v) {
        state = v;
//        state.addListener(this);
//        int numChildren = v.getNumChildren();
//        setSize(getWidth()/4, 40);
    };

    ~ChordComponent() {

    };
    void paint(juce::Graphics &g) override {
        g.fillAll(juce::Colours::white);
        g.setFont(20.0f);
        juce::String chordName = "Right Click To Name Chord";
        if(state.hasProperty(tracktion_engine::IDs::name)){
            chordName = state.getProperty(tracktion_engine::IDs::name).toString();
        }
        if(state.hasProperty(tracktion_engine::IDs::colour)){
            g.setColour(juce::Colour::fromString ( state.getProperty(tracktion_engine::IDs::colour).toString()));
        }

        g.drawText( chordName, getLocalBounds(), juce::Justification::centred, true);
    }

    void resized() override {

    }
    juce::ValueTree state;

    bool testRepaintCalled{false};
private:
    void valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &) override {
        testRepaintCalled = true;
        this->repaint();
    }

    void valueTreeParentChanged(juce::ValueTree &) override {
        this->repaint();
    }
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChordComponent)
};


