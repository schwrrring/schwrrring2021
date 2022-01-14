#pragma once

#include <JuceHeader.h>

// CMake builds don't use an AppConfig.h, so it's safe to include juce module headers
// directly. If you need to remain compatible with Projucer-generated builds, and
// have called `juce_generate_juce_header(<thisTarget>)` in your CMakeLists.txt,
// you could `#include <JuceHeader.h>` here instead, to make all your module headers visible.
#include <juce_gui_extra/juce_gui_extra.h>

#include "common/Utilities.h"
#include "common/Components.h"
//==============================================================================
//==============================================================================
class HeaderComponent : public juce::Component {
public:

    HeaderComponent(tracktion_engine::Engine& en, tracktion_engine::Edit& ed, tracktion_engine::SelectionManager& sm, EditComponent& ec )
            : engine(en), edit(ed), selectionManager(sm), editComponent(ec) {

        settingsButton.onClick = [this] { EngineHelpers::showAudioDeviceSettings(engine); };
        pluginsButton.onClick = [this] {
            DialogWindow::LaunchOptions o;
            o.dialogTitle = TRANS("Plugins");
            o.dialogBackgroundColour = Colours::black;
            o.escapeKeyTriggersCloseButton = true;
            o.useNativeTitleBar = true;
            o.resizable = true;
            o.useBottomRightCornerResizer = true;

            auto v = new PluginListComponent(engine.getPluginManager().pluginFormatManager,
                                             engine.getPluginManager().knownPluginList,
                                             engine.getTemporaryFileManager().getTempFile("PluginScanDeadMansPedal"),
                                             te::getApplicationSettings());
            v->setSize(1400, 1200);
            o.content.setOwned(v);
            o.launchAsync();
        };
        deleteButton.setEnabled(true);

        updatePlayButtonText();
        updateRecordButtonText();


        Helpers::addAndMakeVisible(*this,
                                   {&settingsButton, &pluginsButton, &newEditButton, &playPauseButton, &showEditButton,
                                    &recordButton, &newTrackButton, &deleteButton, &undoButton, &startExcercise,
                                    &importSong
                                    });
        setupButtons();
    }

    void paint(juce::Graphics &g) override {
//                einfach nen rectangle über die ganze weite called Header
        g.fillCheckerBoard(getLocalBounds().toFloat(), 30, 10,
                           juce::Colours::sandybrown, juce::Colours::saddlebrown);

    }

    void resized() override {
        auto r = getLocalBounds();
        int w = getWidth() / 10;
        auto topR = r.removeFromTop(30);
        settingsButton.setBounds(topR.removeFromLeft(w).reduced(2));
        pluginsButton.setBounds(topR.removeFromLeft(w).reduced(2));
        newEditButton.setBounds(topR.removeFromLeft(w).reduced(2));
        playPauseButton.setBounds(topR.removeFromLeft(w).reduced(2));
        recordButton.setBounds(topR.removeFromLeft(w).reduced(2));
        showEditButton.setBounds(topR.removeFromLeft(w).reduced(2));
        newTrackButton.setBounds(topR.removeFromLeft(w).reduced(2));
        deleteButton.setBounds(topR.removeFromLeft(w).reduced(2));
        importSong.setBounds(topR.removeFromLeft(w).reduced(2));
        undoButton.setBounds(topR.removeFromLeft(w).reduced(2));
        startExcercise.setBounds(topR.removeFromLeft(w).reduced(2));
    }

    void updatePlayButtonText() {
        playPauseButton.setButtonText(
                engine.getActiveEdits().getEdits()[0]->getTransport().isPlaying() ? "Stop" : "Play");
    }

    void updateRecordButtonText() {
        recordButton.setButtonText(
                engine.getActiveEdits().getEdits()[0]->getTransport().isRecording() ? "Abort" : "Record");
    }

private:
    tracktion_engine::Engine& engine;
    tracktion_engine::SelectionManager& selectionManager;
    tracktion_engine::Edit& edit;
    EditComponent& editComponent;
    TextButton settingsButton{"Settings"}, pluginsButton{"Plugins"}, newEditButton{"New"}, playPauseButton{"Play"},
            showEditButton{"Show Edit"}, newTrackButton{"New Track"}, deleteButton{"Delete"}, recordButton{"Record"}
    , undoButton{"Undo"}, startExcercise{"Start Excercise"}, importSong{"Import Song.xml"};
    //==============================================================================

    void setupButtons() {
        playPauseButton.onClick = [this] {
            EngineHelpers::togglePlay(*engine.getActiveEdits().getEdits()[0]);
        };
        recordButton.onClick = [this] {
            bool wasRecording = engine.getActiveEdits().getEdits()[0]->getTransport().isRecording();
            EngineHelpers::toggleRecord(*engine.getActiveEdits().getEdits()[0]);
            if (wasRecording)
                te::EditFileOperations(*engine.getActiveEdits().getEdits()[0]).save(true, true, false);
        };
        newTrackButton.onClick = [this] {
            engine.getActiveEdits().getEdits()[0]->ensureNumberOfAudioTracks(
                    getAudioTracks(*engine.getActiveEdits().getEdits()[0]).size() + 1);
        };
        deleteButton.onClick = [this] {
            auto sel = selectionManager.getSelectedObject(0);
            if (auto clip = dynamic_cast<te::Clip *> (sel)) {
                clip->removeFromParentTrack();
            } else if (auto track = dynamic_cast<te::Track *> (sel)) {
                if (!(track->isMarkerTrack() || track->isTempoTrack() || track->isChordTrack()))
                    engine.getActiveEdits().getEdits()[0]->deleteTrack(track);
            } else if (auto plugin = dynamic_cast<te::Plugin *> (sel)) {
                plugin->deleteFromParent();
            }
        };
        importSong.onClick = [this]{
            int indexNewTrack = getAudioTracks(*engine.getActiveEdits().getEdits()[0]).size() + 1;
            engine.getActiveEdits().getEdits()[0]->ensureNumberOfAudioTracks(indexNewTrack);
            auto edit = engine.getActiveEdits().getEdits()[0];
            Helpers::loadChordsFromMusicXmlIntoMidiTrack(* tracktion_engine::getAudioTracks(*edit)[indexNewTrack], *edit, "/Users/malte/Arschloch/GuiApp/hello_world_test/Tests/fixtures/musicXml1.xml");
        };

        undoButton.onClick = [this] {
            edit.undo();
            editComponent.repaint();
        };

        startExcercise.onClick = [this] {
            // Todo: check and find out, why it destroys the undo-Manager
            //            engine.getActiveEdits().getEdits()[0]->ensureNumberOfAudioTracks(getAudioTracks(*engine.getActiveEdits().getEdits()[0]).size() + 1);
//            auto track = getAudioTracks(*engine.getActiveEdits().getEdits()[0])[-1];
//            engine.getActiveEdits().getEdits()[0];
//            for (auto instance : edit.getAllInputDevices()) {
//                if (instance->getInputDevice().getDeviceType() == te::InputDevice::physicalMidiDevice) {
//                    if (auto t = EngineHelpers::getOrInsertAudioTrackAt( *(engine.getActiveEdits().getEdits()[0]), te::getAudioTracks(edit).size())) {
//                        instance->setTargetTrack(*t, 0, true);
//                        instance->setRecordingEnabled(*t, true);
//                    }
//                }
//            }
//            auto clips = EngineHelpers::getOrInsertAudioTrackAt( *(engine.getActiveEdits().getEdits()[0]), 0)->getClips();
//
//            edit.restartPlayback();
//            auto sdf =  &edit;
//
//            bool wasRecording = engine.getActiveEdits().getEdits()[0]->getTransport().isRecording();
//            EngineHelpers::toggleRecord(edit);
//            if (wasRecording)
//                te::EditFileOperations(*engine.getActiveEdits().getEdits()[0]).save(true, true, false);

        };
//            showWaveformButton.onClick = [this] {
//                auto &evs = editComponent->getEditViewState();
//                evs.drawWaveforms = !evs.drawWaveforms.get();
//                showWaveformButton.setToggleState(evs.drawWaveforms, dontSendNotification);
//            };
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderComponent)
};
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
