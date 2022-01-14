//
// Created by Malte Hildebrand on 25.09.21.
//


#pragma once

#include <JuceHeader.h>
#include "ChordComponent.h"


//==============================================================================
//==============================================================================
class ChordSheetComponent : public juce::Component, private juce::ValueTree::Listener {
public:

    ChordSheetComponent() {
        setSize(400, 400);
        paintChordComponents(state);
    }
    ChordSheetComponent(juce::ValueTree v) {
        state = v;
        state.addListener(this);
        setSize(400, 400);
        paintChordComponents(state);
    }

    void paintChordComponents(const juce::ValueTree & trackTree) {
        juce::ValueTree valueTree(trackTree);
        this->removeAllChildren();
        int numChildren = valueTree.getNumChildren();
        for (int i = 0; i < numChildren; ++i) {
            if (valueTree.getChild(i).hasType(tracktion_engine::IDs::MIDICLIP)) {
                addNewChord(valueTree.getChild(i));
            }
        }

    };

    ~ChordSheetComponent() {

    };

    void paint(juce::Graphics &g) override {
        g.fillAll(juce::Colours::white);
        g.setColour(juce::Colours::darkblue);
        g.setFont(20.0f);
        g.drawText("Hello, World!", getLocalBounds(), juce::Justification::centred, true);
        int numChildrenComponents = this->getNumChildComponents();
        for (int i = 0; i < numChildrenComponents; ++i) {
            int yValue;
            if(i>0){
                yValue =  (std::floor(i/4)) * 40;
            }
            else{ yValue=0; }
            int xValue;
            if(i>0){
                xValue =  getWidth() / 4 *  (i -  std::floor( i/4) * 4);
            }
            else{ xValue=0; }

            this->getChildComponent(i)->setBounds(xValue, yValue, getWidth() / 4, 20);
        }
    }

    void resized() override {

        int numChildrenComponents = this->getNumChildComponents();
        for (int i = 0; i < numChildrenComponents; ++i) {
            int yValue;
            if(i>0){
                yValue =  (std::floor(i/4)) * 40;
            }
            else{ yValue=0; }
            int xValue;
            if(i>0){
               xValue =  getWidth() / 4 *  (i -  std::floor( i/4) * 4);
            }
            else{ xValue=0; }

            this->getChildComponent(i)->setBounds(xValue, yValue, getWidth() / 4, 20);
        }
    }

    void addNewChord(juce::ValueTree midiClipValueTree) {
        auto chord = components.add(std::make_unique<ChordComponent>(midiClipValueTree));
        addAndMakeVisible(*chord);

    }

    void valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &) override {
        testPaintChordComonentsCalled = true;
//        paintChordComponents(state);
        paintChordComponents(state);
        this->repaint();
    }
    void valueTreeChildRemoved(juce::ValueTree &parentTree, juce::ValueTree &childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override{
        paintChordComponents(state);
        this->repaint();
    }
    void valueTreeChildAdded(juce::ValueTree &parentTree, juce::ValueTree &childWhichHasBeenAdded) override{
        paintChordComponents(state);
        this->repaint();
    }

    bool testPaintChordComonentsCalled{false};
    juce::ValueTree state;
private:

    juce::OwnedArray<ChordComponent> components;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChordSheetComponent)
};

