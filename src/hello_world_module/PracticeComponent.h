//
// Created by Malte Hildebrand on 02.09.21.
//

#ifndef GUI_APP_EXAMPLE_2_MIDIRECORDING0209_H
#define GUI_APP_EXAMPLE_2_MIDIRECORDING0209_H

#pragma once

#include <JuceHeader.h>
#include <hello_world_module/common/PluginWindow.h>

//==============================================================================
class PracticeComponent :
        public Component,
        private ChangeListener,
        private juce::MidiInputCallback,
        private juce::MidiKeyboardStateListener {
public:
    //==============================================================================
    PracticeComponent() {

        File projectFile = loadProjectFromFile();
        setUpMidiInput();
        selectionManager.addChangeListener(this);
        keyboardState.addListener(this);
        headerComponent = std::make_unique<HeaderComponent>(engine, *edit, selectionManager, *editComponent);
        setUpButtonsAndLabel(projectFile);
        setupNextLesson();
        setSize(800, 800);
    }


    void setupNextLesson() {
        currentLesson = Helpers::getNextLesson(*edit);
        currentExcercise = Helpers::getNextClip(currentLesson);
        if (currentExcercise != NULL) {
            ValueTree trackState = currentLesson->state;
            chordSheet = std::__1::make_unique<ChordSheetComponent>(trackState);
            addAndMakeVisible(*chordSheet);
            Helpers::colourClipsWithPracticeTimesInTheFutureYellow(*edit);
            copyOfClipSequence.copyFrom(currentExcercise->getSequence(), nullptr);
        }
    }

    File loadProjectFromFile() {
        auto d = File::getSpecialLocation(File::tempDirectory).getChildFile("MaltesMuddi");
        d.createDirectory();

        auto f = Helpers::findRecentEdit(d);
        if (f.existsAsFile())
            createOrLoadEdit(f);
        else
            createOrLoadEdit(d.getNonexistentChildFile("Test", ".tracktionedit", false));
        return f;
    }

    void setUpButtonsAndLabel(const File &f) {
        Helpers::addAndMakeVisible(*this,
                                   {&midiMessagesBox, &midiInputList}
        );

        for (auto childComp: headerComponent->getChildren()) {
            if (auto *textButton = dynamic_cast<TextButton *> (childComp)) {
                if (textButton->getButtonText().equalsIgnoreCase("New")) {
                    textButton->onClick = [this] { createOrLoadEdit(); };
                };
            }
        }
        addAndMakeVisible(*headerComponent);

        midiMessagesBox.setMultiLine(true);
        midiMessagesBox.setReturnKeyStartsNewLine(true);
        midiMessagesBox.setReadOnly(true);
        midiMessagesBox.setScrollbarsShown(true);
        midiMessagesBox.setCaretVisible(false);
        midiMessagesBox.setPopupMenuEnabled(true);
        midiMessagesBox.setColour(TextEditor::backgroundColourId, Colour(0x32ffffff));
        midiMessagesBox.setColour(TextEditor::outlineColourId, Colour(0x1c000000));
        midiMessagesBox.setColour(TextEditor::shadowColourId, Colour(0x16000000));
        editNameLabel.setJustificationType(Justification::centred);
        midiInputListLabel.setText("MIDI Input:", dontSendNotification);
        midiInputListLabel.attachToComponent(&midiInputList, true);
        midiInputList.setTextWhenNoChoicesAvailable("No MIDI Inputs Enabled");
        for (auto childComp: headerComponent->getChildren()) {
            if (auto *textButton = dynamic_cast<TextButton *> (childComp)) {
                if (textButton->getButtonText().equalsIgnoreCase("Show Edit")) {
                    textButton->onClick = [this, f] {
                        te::EditFileOperations(*edit).save(true, true, false);
                        f.revealToUser();
                    };
                };
            }
        }
        headerComponent->updatePlayButtonText();
        headerComponent->updateRecordButtonText();
    }

    void setUpMidiInput() {
        auto midiInputs = MidiInput::getAvailableDevices();
        StringArray midiInputNames;
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
    }


    ~PracticeComponent() {
        te::EditFileOperations(*edit).save(true, true, false);
        engine.getTemporaryFileManager().getTempDirectory().deleteRecursively();
    }

    //==============================================================================

    void paint(Graphics &g) override {
        g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    }

    void resized() override {
        auto r = getLocalBounds();
        auto topR = r.removeFromTop(30).reduced(2);
//        midiInputList->setBounds(r.removeFromTop(30).reduced(2));
        headerComponent->setBounds(topR);
        midiInputList.setBounds(r.removeFromTop(30).reduced(2));
        chordSheet->setBounds(r.removeFromTop(400).reduced(2));
        midiMessagesBox.setBounds(r.removeFromTop(80).reduced(2));

        if (editComponent != nullptr)
            editComponent->setBounds(r);
    }

private:
    //==============================================================================
    tracktion_engine::Engine engine{"EasyEars", std::make_unique<ExtendedUIBehaviour>(), nullptr};
    te::SelectionManager selectionManager{engine};

    TextButton button{"test"};
    std::unique_ptr <te::Edit> edit;
    std::unique_ptr <EditComponent> editComponent;
//  todo:  hier mache ich den fehler, die Adresse hinter einem Pointer zu übergeben, bei dem kein Wert i
    std::unique_ptr <HeaderComponent> headerComponent;
    int lastInputIndex = 0;                           // [3]
    juce::ComboBox midiInputList;                     // [2]
    juce::Label midiInputListLabel;
    Label editNameLabel{"No Edit Loaded"};
    bool isAddingFromMidiInput = false;               // [4]
    juce::MidiKeyboardState keyboardState;
    std::unique_ptr <ChordSheetComponent> chordSheet;   // [5]
    ToggleButton showWaveformButton{"Show Waveforms"};
//    ToggleButton showWaveformButton{"Show Waveforms"};

    juce::AudioDeviceManager deviceManager;           // [1]
    juce::TextEditor midiMessagesBox;
    tracktion_engine::AudioTrack *currentLesson;
    tracktion_engine::MidiClip *currentExcercise;
    tracktion_engine::MidiList copyOfClipSequence{};


    //==============================================================================
    // These methods handle callbacks from the midi device + on-screen keyboard..
    void handleIncomingMidiMessage(juce::MidiInput *source, const juce::MidiMessage &message) override {
        const juce::ScopedValueSetter<bool> scopedInputFlag(isAddingFromMidiInput, true);
        if (!edit->getTransport().isRecording()) {
            if (currentExcercise != NULL) {
                postToChordCheckerList(keyboardState, currentExcercise, message);
            }
        }
        postMessageToList(message, source->getName());
    }


    // This is used to dispach an incoming message to the message thread
    class CheckChordCallback : public juce::CallbackMessage {
    public:
        CheckChordCallback(PracticeComponent *o,
                           const juce::MidiMessage &m,
                           juce::MidiKeyboardState &k,
                           tracktion_engine::MidiClip *c,
                           tracktion_engine::MidiList *l)
                : owner(o), message(m), clip(c), keyboardState(k), midilist(l) {
        }

        void messageCallback() override {
            if (owner != nullptr)
                // here nen switch reinbauen, polyphonic midi checker monophonic checker
                if (false) {
                    owner->handleChord(keyboardState, clip, message);
                } else {
                    owner->handleArpeggio(keyboardState, clip,
                                          message, midilist);
                }
        }

        Component::SafePointer <PracticeComponent> owner;
        juce::MidiMessage message;
        tracktion_engine::MidiClip *clip;
        tracktion_engine::MidiList *midilist;
        juce::MidiKeyboardState &keyboardState;
    };

// todo: this zurückgeben um die checker zu chainen
    void handleChord(juce::MidiKeyboardState &keyboardState, tracktion_engine::MidiClip *clip,
                     const juce::MidiMessage &message) {

        auto playedCorrect = Helpers::checkChord(keyboardState, currentExcercise, message);
        if (playedCorrect) {
            Helpers::colourClip(currentExcercise, Colours::green);
            editComponent->repaint();
            Helpers::setIsPlayed(currentExcercise);
            if (Helpers::isExerciseCompleted(currentLesson)) {
                Helpers::resetExerciseForNextPractice(currentLesson);
                Helpers::setNextPracticeTime(currentLesson);
                chordSheet->state = Helpers::getNextLesson(*edit)->state;
                chordSheet->paintChordComponents(Helpers::getNextLesson(*edit)->state);
                Helpers::colourClipsWithPracticeTimesInTheFutureYellow(*edit);
                currentLesson = Helpers::getNextLesson(*edit);
            }
            currentExcercise = Helpers::getNextClip(currentLesson);
            copyOfClipSequence.copyFrom(currentExcercise->getSequence(), nullptr);
        }
    }

    void handleArpeggio(juce::MidiKeyboardState &keyboardState, tracktion_engine::MidiClip *clip,
                        const juce::MidiMessage &message, tracktion_engine::MidiList *midiList) {
        auto playedCorrectly = Helpers::handleArpeggio(keyboardState, clip,
                                                       message, midiList);
        if (playedCorrectly) {
            Helpers::colourClip(currentExcercise, Colours::green);
            editComponent->repaint();
            Helpers::setIsPlayed(currentExcercise);
            if (Helpers::isExerciseCompleted(currentLesson)) {
                Helpers::resetExerciseForNextPractice(currentLesson);
                Helpers::setNextPracticeTime(currentLesson);
                chordSheet->state = Helpers::getNextLesson(*edit)->state;
                chordSheet->paintChordComponents(Helpers::getNextLesson(*edit)->state);
                Helpers::colourClipsWithPracticeTimesInTheFutureYellow(*edit);
                currentLesson = Helpers::getNextLesson(*edit);
            }
            currentExcercise = Helpers::getNextClip(currentLesson);
            copyOfClipSequence.copyFrom(currentExcercise->getSequence(), nullptr);
        }
    }

    void setNextPracticeTime(tracktion_engine::Track *track) {
        track->state.removeProperty(schwrrring::IDs::isNewExercise, &edit->getUndoManager());
        if (!track->state.hasProperty(schwrrring::IDs::nextPracticeTime)) {
            PracticeTimes practiceTimes;
            long retVal = Helpers::getUnixTimeStamp();
            practiceTimes.times.add(retVal + 30L);
            track->state.setProperty(schwrrring::IDs::nextPracticeTime, (juce::String) practiceTimes.toString(),
                                     &edit->getUndoManager());
        } else {
            PracticeTimes *practiceTimes = new PracticeTimes(
                    track->state.getProperty(schwrrring::IDs::nextPracticeTime));
            long lastPracticeTime = practiceTimes->times.getLast();
            practiceTimes->times.add(lastPracticeTime + 30);
            track->state.setProperty(schwrrring::IDs::nextPracticeTime, practiceTimes->toString(),
                                     &edit->getUndoManager());
        }
    }

    bool comparetime(juce::String time1) {
        std::tm tm = {};
        auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        auto now = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = now - tp;
        DBG("secondCounts:");
        DBG((juce::String) elapsed_seconds.count());
        return elapsed_seconds.count() > 0;
    }

    void handleNoteOn(juce::MidiKeyboardState *, int midiChannel, int midiNoteNumber, float velocity) override {
        if (isAddingFromMidiInput) {
            auto m = juce::MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity);
            // todo:    add m in einen State

        }
        if (!isAddingFromMidiInput) {
            auto m = juce::MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity);
            m.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
            postMessageToList(m, "On-Screen Keyboard");
        }
    }

    void handleNoteOff(juce::MidiKeyboardState *, int midiChannel, int midiNoteNumber, float /*velocity*/) override {
        if (isAddingFromMidiInput) {
            auto m = juce::MidiMessage::noteOff(midiChannel, midiNoteNumber);
            // todo:    remove m aus einem State
        }

        if (!isAddingFromMidiInput) {
            auto m = juce::MidiMessage::noteOff(midiChannel, midiNoteNumber);
            m.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
            postMessageToList(m, "On-Screen Keyboard");
        }
    }


    void postMessageToList(const juce::MidiMessage &message, const juce::String &source) {
        (new IncomingMessageCallback(this, message, source))->post();
    }


    void postToChordCheckerList(juce::MidiKeyboardState &keyboardState, tracktion_engine::MidiClip *clip,
                                const juce::MidiMessage &message) {
        (new CheckChordCallback(this, message, keyboardState, clip, &copyOfClipSequence))->post();
    }

    // This is used to dispach an incoming message to the message thread
    class IncomingMessageCallback : public juce::CallbackMessage {
    public:
        IncomingMessageCallback(PracticeComponent *o, const juce::MidiMessage &m, const juce::String &s)
                : owner(o), message(m), source(s) {}

        void messageCallback() override {
            if (owner != nullptr)
                owner->addMessageToList(message, source);

        }

        Component::SafePointer <PracticeComponent> owner;
        juce::MidiMessage message;
        juce::String source;
    };


    void addMessageToList(const juce::MidiMessage &message, const juce::String &source) {
        auto description = getMidiMessageDescription(message);

        juce::String midiMessageString(description + " (" + source + ")"); // [7]
        logMessage(midiMessageString);
    }

    static juce::String getMidiMessageDescription(const juce::MidiMessage &m) {
        if (m.isNoteOn()) return "Note on " + juce::MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3);
        if (m.isNoteOff()) return "Note off " + juce::MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3);
        if (m.isProgramChange()) return "Program change " + juce::String(m.getProgramChangeNumber());
        if (m.isPitchWheel()) return "Pitch wheel " + juce::String(m.getPitchWheelValue());
        if (m.isAftertouch())
            return "After touch " + juce::MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3) + ": " +
                   juce::String(m.getAfterTouchValue());
        if (m.isChannelPressure()) return "Channel pressure " + juce::String(m.getChannelPressureValue());
        if (m.isAllNotesOff()) return "All notes off";
        if (m.isAllSoundOff()) return "All sound off";
        if (m.isMetaEvent()) return "Meta event";

        if (m.isController()) {
            juce::String name(juce::MidiMessage::getControllerName(m.getControllerNumber()));

            if (name.isEmpty())
                name = "[" + juce::String(m.getControllerNumber()) + "]";

            return "Controller " + name + ": " + juce::String(m.getControllerValue());
        }

        return juce::String::toHexString(m.getRawData(), m.getRawDataSize());
    }


    void createOrLoadEdit(File editFile = {}) {
        if (editFile == File()) {
            FileChooser fc("New Edit", File::getSpecialLocation(File::userDocumentsDirectory), "*.tracktionedit");
            if (fc.browseForFileToSave(true))
                editFile = fc.getResult();
            else
                return;
        }

        selectionManager.deselectAll();
        editComponent = nullptr;

        if (editFile.existsAsFile())
            edit = te::loadEditFromFile(engine, editFile);
        else
            edit = te::createEmptyEdit(engine, editFile);

        edit->editFileRetriever = [editFile] { return editFile; };
        edit->playInStopEnabled = true;

        auto &transport = edit->getTransport();
        transport.addChangeListener(this);

        editNameLabel.setText(editFile.getFileNameWithoutExtension(), dontSendNotification);

        createTracksAndAssignInputs();

        te::EditFileOperations(*edit).save(true, true, false);

        editComponent = std::make_unique<EditComponent>(*edit, selectionManager);
        editComponent->getEditViewState().showFooters = true;
        editComponent->getEditViewState().showMidiDevices = true;
        editComponent->getEditViewState().showWaveDevices = true;
        editComponent->getEditViewState().showChordTrack = false;
        editComponent->getEditViewState().showArrangerTrack = false;
        editComponent->getEditViewState().showChordTrack = false;
        editComponent->getEditViewState().showGlobalTrack = false;


        editComponent->setSize(20, 20);

        addAndMakeVisible(*editComponent);
    }

    void logMessage(const juce::String &m) {
        midiMessagesBox.moveCaretToEnd();
        midiMessagesBox.insertTextAtCaret(m + juce::newLine);
    }

    void setMidiInput(int index) {
        auto list = juce::MidiInput::getAvailableDevices();

        deviceManager.removeMidiInputDeviceCallback(list[lastInputIndex].identifier, this);

        auto newInput = list[index];

        if (!deviceManager.isMidiInputDeviceEnabled(newInput.identifier))
            deviceManager.setMidiInputDeviceEnabled(newInput.identifier, true);

        deviceManager.addMidiInputDeviceCallback(newInput.identifier, this);
        midiInputList.setSelectedId(index + 1, juce::dontSendNotification);

        lastInputIndex = index;
    }

    void changeListenerCallback(ChangeBroadcaster *source) override {
        if (edit != nullptr && source == &edit->getTransport()) {
            headerComponent->updatePlayButtonText();
            headerComponent->updateRecordButtonText();
        } else if (source == &selectionManager) {
            auto sel = selectionManager.getSelectedObject(0);

//            todo: reactivate it;
//            deleteButton.setEnabled(dynamic_cast<te::Clip *> (sel) != nullptr
//            || dynamic_cast<te::Track *> (sel) != nullptr
//            || dynamic_cast<te::Plugin *> (sel));
        }
    }

    void createTracksAndAssignInputs() {

        auto &dm = engine.getDeviceManager();

        for (int i = 0; i < dm.getNumMidiInDevices(); i++) {
            if (auto mip = dm.getMidiInDevice(i)) {
                mip->setEndToEndEnabled(true);
                mip->setEnabled(true);
            }
        }

        edit->getTransport().ensureContextAllocated();

        if (te::getAudioTracks(*edit).size() == 0) {
            int trackNum = 0;
            for (auto instance : edit->getAllInputDevices()) {
                if (instance->getInputDevice().getDeviceType() == te::InputDevice::physicalMidiDevice) {
                    if (auto t = EngineHelpers::getOrInsertAudioTrackAt(*edit, trackNum)) {
                        instance->setTargetTrack(*t, 0, true);
                        instance->setRecordingEnabled(*t, true);
                        trackNum++;
                    }
                }
            }
        }

        edit->restartPlayback();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PracticeComponent)
};


#endif //GUI_APP_EXAMPLE_2_MIDIRECORDING0209_H
