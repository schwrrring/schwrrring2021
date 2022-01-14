/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2018
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com
*/

#pragma once

#include <hello_world_module/Components/ChordSheetComponent.h>
#include <mx/api/ApiCommon.h>
#include <mx/api/DocumentManager.h>
#include "../PracticeTimes.h"
#include "ChordClipMapper.h"

namespace te = tracktion_engine;

//==============================================================================


//==============================================================================
namespace PlayHeadHelpers {
    // Quick-and-dirty function to format a timecode string
    static inline juce::String timeToTimecodeString(double seconds) {
        auto millisecs = roundToInt(seconds * 1000.0);
        auto absMillisecs = std::abs(millisecs);

        return juce::String::formatted("%02d:%02d:%02d.%03d",
                                       millisecs / 3600000,
                                       (absMillisecs / 60000) % 60,
                                       (absMillisecs / 1000) % 60,
                                       absMillisecs % 1000);
    }

    // Quick-and-dirty function to format a bars/beats string
    static inline juce::String
    quarterNotePositionToBarsBeatsString(double quarterNotes, int numerator, int denominator) {
        if (numerator == 0 || denominator == 0)
            return "1|1|000";

        auto quarterNotesPerBar = ((double) numerator * 4.0 / (double) denominator);
        auto beats = (fmod(quarterNotes, quarterNotesPerBar) / quarterNotesPerBar) * numerator;

        auto bar = ((int) quarterNotes) / quarterNotesPerBar + 1;
        auto beat = ((int) beats) + 1;
        auto ticks = ((int) (fmod(beats, 1.0) * 960.0 + 0.5));

        return juce::String::formatted("%d|%d|%03d", bar, beat, ticks);
    }

    // Returns a textual description of a CurrentPositionInfo
    static inline juce::String getTimecodeDisplay(const AudioPlayHead::CurrentPositionInfo &pos) {
        MemoryOutputStream displayText;

        displayText << juce::String(pos.bpm, 2) << " bpm, "
                    << pos.timeSigNumerator << '/' << pos.timeSigDenominator
                    << "  -  " << timeToTimecodeString(pos.timeInSeconds)
                    << "  -  " << quarterNotePositionToBarsBeatsString(pos.ppqPosition,
                                                                       pos.timeSigNumerator,
                                                                       pos.timeSigDenominator);

        if (pos.isRecording)
            displayText << "  (recording)";
        else if (pos.isPlaying)
            displayText << "  (playing)";
        else
            displayText << "  (stopped)";

        return displayText.toString();
    }
}

//==============================================================================
namespace EngineHelpers {
    te::Project::Ptr createTempProject(te::Engine &engine) {
        auto file = engine.getTemporaryFileManager().getTempDirectory().getChildFile("temp_project").withFileExtension(
                te::projectFileSuffix);
        te::ProjectManager::TempProject tempProject(engine.getProjectManager(), file, true);
        return tempProject.project;
    }

    void showAudioDeviceSettings(te::Engine &engine) {
        DialogWindow::LaunchOptions o;
        o.dialogTitle = TRANS("Audio Settings");
        o.dialogBackgroundColour = LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId);
        o.content.setOwned(new AudioDeviceSelectorComponent(engine.getDeviceManager().deviceManager,
                                                            0, 512, 1, 512, false, false, true, true));
        o.content->setSize(400, 600);
        o.launchAsync();
    }

    void browseForAudioFile(te::Engine &engine, std::function<void(const File &)> fileChosenCallback) {
        auto fc = std::make_shared<FileChooser>("Please select an audio file to load...",
                                                engine.getPropertyStorage().getDefaultLoadSaveDirectory(
                                                        "pitchAndTimeExample"),
                                                engine.getAudioFileFormatManager().readFormatManager.getWildcardForAllFormats());

        fc->launchAsync(FileBrowserComponent::openMode + FileBrowserComponent::canSelectFiles,
                        [fc, &engine, callback = std::move(fileChosenCallback)](const FileChooser &) {
                            const auto f = fc->getResult();

                            if (f.existsAsFile())
                                engine.getPropertyStorage().setDefaultLoadSaveDirectory("pitchAndTimeExample",
                                                                                        f.getParentDirectory());

                            callback(f);
                        });
    }

    void removeAllClips(te::AudioTrack &track) {
        auto clips = track.getClips();

        for (int i = clips.size(); --i >= 0;)
            clips.getUnchecked(i)->removeFromParentTrack();
    }

    te::AudioTrack *getOrInsertAudioTrackAt(te::Edit &edit, int index) {
        edit.ensureNumberOfAudioTracks(index + 1);
        return te::getAudioTracks(edit)[index];
    }

    te::WaveAudioClip::Ptr loadAudioFileAsClip(te::Edit &edit, const File &file) {
        // Find the first track and delete all clips from it
        if (auto track = getOrInsertAudioTrackAt(edit, 0)) {
            removeAllClips(*track);

            // Add a new clip to this track
            te::AudioFile audioFile(edit.engine, file);

            if (audioFile.isValid())
                if (auto newClip = track->insertWaveClip(file.getFileNameWithoutExtension(), file,
                                                         {{0.0, audioFile.getLength()}, 0.0}, false))
                    return newClip;
        }

        return {};
    }

    template<typename ClipType>
    typename ClipType::Ptr loopAroundClip(ClipType &clip) {
        auto &transport = clip.edit.getTransport();
        transport.setLoopRange(clip.getEditTimeRange());
        transport.looping = true;
        transport.position = 0.0;
        transport.play(false);

        return clip;
    }

    void togglePlay(te::Edit &edit) {
        auto &transport = edit.getTransport();

        if (transport.isPlaying())
            transport.stop(false, false);
        else
            transport.play(false);
    }

    void toggleRecord(te::Edit &edit) {
        auto &transport = edit.getTransport();

        if (transport.isRecording())
            transport.stop(true, false);
        else
            transport.record(false);
    }

    void armTrack(te::AudioTrack &t, bool arm, int position = 0) {
        auto &edit = t.edit;
        for (auto instance : edit.getAllInputDevices())
            if (instance->isOnTargetTrack(t, position))
                instance->setRecordingEnabled(t, arm);
    }

    bool isTrackArmed(te::AudioTrack &t, int position = 0) {
        auto &edit = t.edit;
        for (auto instance : edit.getAllInputDevices())
            if (instance->isOnTargetTrack(t, position))
                return instance->isRecordingEnabled(t);

        return false;
    }

    bool isInputMonitoringEnabled(te::AudioTrack &t, int position = 0) {
        auto &edit = t.edit;
        for (auto instance : edit.getAllInputDevices())
            if (instance->isOnTargetTrack(t, position))
                return instance->getInputDevice().isEndToEndEnabled();

        return false;
    }

    void enableInputMonitoring(te::AudioTrack &t, bool im, int position = 0) {
        if (isInputMonitoringEnabled(t, position) != im) {
            auto &edit = t.edit;
            for (auto instance : edit.getAllInputDevices())
                if (instance->isOnTargetTrack(t, position))
                    instance->getInputDevice().flipEndToEnd();
        }
    }

    bool trackHasInput(te::AudioTrack &t, int position = 0) {
        auto &edit = t.edit;
        for (auto instance : edit.getAllInputDevices())
            if (instance->isOnTargetTrack(t, position))
                return true;

        return false;
    }

    inline std::unique_ptr<juce::KnownPluginList::PluginTree> createPluginTree(te::Engine &engine) {
        auto &list = engine.getPluginManager().knownPluginList;

        if (auto tree = list.createTree(list.getTypes(), KnownPluginList::sortByManufacturer))
            return tree;

        return {};
    }

}
namespace Helpers {
    static inline void addAndMakeVisible(juce::Component &parent, const juce::Array<juce::Component *> &children) {
        for (auto c : children)
            parent.addAndMakeVisible(c);
    }

    static inline juce::String
    getStringOrDefault(const juce::String &stringToTest, const juce::String &stringToReturnIfEmpty) {
        return stringToTest.isEmpty() ? stringToReturnIfEmpty : stringToTest;
    }

    static inline juce::XmlDocument readXmlFile(juce::String path) {
        juce::File file = juce::File{path};
        return XmlDocument(file);
    }

    static inline std::vector<juce::XmlElement> getParts(XmlElement &xmlElement) {
        xmlElement.getTagName();
        std::vector<juce::XmlElement> parts{};

        if (xmlElement.hasTagName("score-partwise")) {
            // now we'll iterate its sub-elements looking for 'giraffe' elements..
            for (auto *e : xmlElement.getChildIterator()) {
                if (e->hasTagName("part")) {
                    parts.push_back(*e);
                }
            }
        }
        return parts;
    }

    static inline std::vector<juce::XmlElement> getMeasures(XmlElement &xmlElement) {
        xmlElement.getTagName();
        std::vector<juce::XmlElement> parts{};

        if (xmlElement.hasTagName("part")) {
            // now we'll iterate its sub-elements looking for 'giraffe' elements..
            for (auto *e : xmlElement.getChildIterator()) {
                if (e->hasTagName("measure")) {
                    parts.push_back(*e);
                }
            }
        }
        return parts;
    }

    std::vector<ChordClipMapper> getChordsWithDurationFromMusicXmlDocument(String path) {
        std::vector<ChordClipMapper> chordsWithDuration{};
        using namespace mx::api;
        using namespace mx::core;

        // create a reference to the singleton which holds documents in memory for us
        auto &mgr = DocumentManager::getInstance();

        // ask the document manager to parse the xml into memory for us, returns a document ID.
        const auto documentID = mgr.createFromFile(path.toStdString());

        // get the structural representation of the score from the document manager
        const auto score = mgr.getData(documentID);

        // we need to explicitly destroy the document from memory
        mgr.destroyDocument(documentID);

        // drill down into the data structure to retrieve the note
        int p = 0;
        for (auto &part : score.parts) {
            int m = 0;
            for (auto measure : part.measures) {

                auto staff = measure.staves.front();
                for (auto direction : staff.directions) {
                    bool isFirstChordInMeasure = true;
                    for (auto chord : direction.chords) {
                        std::cout << chord.rootAlter;
                        //                get length either by calculating diff of ticktime and ticktime of next direction or align note and direction
                        std::string divisionsOfMeasure;
                        int divisions = score.ticksPerQuarter;
                        ChordKind kindText = chord.chordKind;
                        mx::api::Step rootStep = chord.root;
                        int rootAlter = chord.rootAlter;
                        int tickTimeposition = direction.tickTimePosition;
                        //                todo: calculate duration via diff to next direction:
                        int measure = m;
                        double duration = 1;
                        isFirstChordInMeasure = false;
                        //                            if (kindText.isNotEmpty(), rootStep.isNotEmpty(), rootAlter.isNotEmpty(), duration.isNotEmpty(), divisions.isNotEmpty()) {
                        chordsWithDuration.push_back(
                                *(new ChordClipMapper(kindText, rootStep, rootAlter, duration, divisions,
                                                      m, tickTimeposition)));

//                        if (!isFirstChordInMeasure) {
//                            if (chordsWithDuration.size() > 0) {
//                                chordsWithDuration.end()->setDuration(
//                                        direction.tickTimePosition - chordsWithDuration.end()->getTickTimePosition());
//                            }
//                        }
                    }
                }
                m++;
            }
            p++;
        }

        // be aware some might not have chords
        return chordsWithDuration;
    }


    //        xmlElement.getTagName();
    //        std::vector<ChordClipMapper> chordsWithDuration{};
    //        std::vector<XmlElement> parts = getParts(xmlElement);
    //        std::string divisionsOfMeasure;
    //        for (auto part : parts) {
    //            std::vector<XmlElement> measures = getMeasures(part);
    //
    //            for (auto measure : measures) {
    //                if (measure.hasTagName("measure")) {
    //                    // set Divisions For meassure passed on Chord later
    //                    for (auto *harmony : measure.getChildIterator()) {
    //                        if (harmony->hasTagName("attributes")) {
    //                            for (auto *ed : harmony->getChildIterator()) {
    //                                if (ed->hasTagName("divisions")) {
    //                                    divisionsOfMeasure = ed->getAllSubText();
    //                                }
    //                            }
    //                        }
    //                        if (harmony->hasTagName("harmony")) {
    //                            juce::String divisions = divisionsOfMeasure;
    //                            juce::String kindText;
    //                            juce::String rootStep;
    //                            juce::String rootAlter;
    //                            juce::String duration;
    //
    //                            for (auto *e : harmony->getChildIterator()) {
    //                                if (e->hasTagName("kind")) {
    //                                    kindText = e->getAllSubText();
    //                                }
    //                                if (e->hasTagName("root")) {
    //                                    for (auto *e : e->getChildIterator()) {
    //                                        if (e->hasTagName("root-step")) {
    //                                            rootStep = e->getAllSubText();
    //                                        }
    //                                        if (e->hasTagName("root-alter")) {
    //
    //                                            rootAlter = e->getAllSubText();
    //                                        }
    //                                    }
    //                                }
    //                                if (auto note = harmony->getNextElementWithTagName("note")) {
    //                                    for (auto *en : note->getChildIterator()) {
    //                                        if (en->hasTagName("duration")) {
    //                                            duration = en->getAllSubText();
    //                                        }
    //                                    }
    //                                }
    //                            }
    //
    //                            if (kindText.isNotEmpty(), rootStep.isNotEmpty(), rootAlter.isNotEmpty(), duration.isNotEmpty(), divisions.isNotEmpty()) {
    //                                chordsWithDuration.push_back(
    //                                        *(new ChordClipMapper(kindText, rootStep, rootAlter, duration, divisions,
    //                                                              "1")));
    //                            }
    //                        }
    //                    }
    //                }
    //            }
    //        }
    //
    //        return chordsWithDuration;


    //    static void playWithmMx() {
    //        using namespace mx::api;
    //
    //        // create a reference to the singleton which holds documents in memory for us
    //        auto &mgr = DocumentManager::getInstance();
    //
    //
    //
    //        // ask the document manager to parse the xml into memory for us, returns a document ID.
    //        const auto documentID = mgr.createFromFile(
    //                "/Users/malte/Arschloch/GuiApp/hello_world_test/Tests/fixtures/musicXml1.xml");
    //
    //        // get the structural representation of the score from the document manager
    //        const auto score = mgr.getData(documentID);
    //
    //        // we need to explicitly destroy the document from memory
    //        mgr.destroyDocument(documentID);
    //
    //        // make sure we have exactly one part
    ////        if( score.parts.size() != 1 )
    ////        {
    ////            return MX_IS_A_FAILURE;
    ////        }
    //
    //        // drill down into the data structure to retrieve the note
    //        const auto &part = score.parts.at(0);
    //        const auto &measure = part.measures.at(0);
    //        const auto &staff = measure.staves.at(0);
    //        const auto &voice = staff.voices.at(0);
    //        const auto &note = voice.notes.at(0);
    //    }
    //    static void BuildChordChordClipFromMusicXML(String path) {
    //        ChordClipMapper *chordClipMapper = new ChordClipMapper();
    //        XmlDocument rootElement = readXmlFile(path);
    //        std::vector<XmlElement> measures = getMeasures(*rootElement.getDocumentElement());
    //        for (auto measure : measures) {
    //            std::vector<ChordClipMapper> chordClipMappers = getChordsWithDurationFromMeasure(measure);
    //        }
    //    }


    static inline juce::File findRecentEdit(const juce::File &dir) {
        auto files = dir.findChildFiles(juce::File::findFiles, false, "*.tracktionedit");

        if (files.size() > 0) {
            files.sort();
            return files.getLast();
        }

        return {};
    }

    static inline long getUnixTimeStamp(const std::time_t *t = nullptr) {
        //if specific time is not passed then get current time
        std::time_t st = t == nullptr ? std::time(nullptr) : *t;
        auto secs = static_cast<std::chrono::milliseconds>(st).count();
        return static_cast<long>(secs);
    }

    static inline void
    loadChordsFromMusicXmlIntoMidiTrack(tracktion_engine::AudioTrack &track, tracktion_engine::Edit &edit, String path) {
//        todo: hardgecodeten path durch variable und select menu ersetzen
        juce::XmlDocument retValXmlDoc = Helpers::readXmlFile(
                path);
        std::vector<ChordClipMapper> chords = Helpers::getChordsWithDurationFromMusicXmlDocument(
                path);

        for (auto chord : chords) {
//            Todo: b set Bars chorrecly
            const te::EditTimeRange editTimeRange = chord.calculateBarsAndBeatsForChord(edit);
//            const te::EditTimeRange editTimeRange = tracktion_engine::EditTimeRange(edit.tempoSequence.barsBeatsToTime(
//                                                                                            {0, chord.getDuration()}),
//                                                                                    edit.tempoSequence.barsBeatsToTime(
//                                                                                            {0, 4.0}));
            track.insertNewClip(te::TrackItem::Type::midi, "MIDI Clip", editTimeRange, nullptr);
            auto midiClip = dynamic_cast<te::MidiClip *> (track.getClips()[0]);
            auto &pg = *midiClip->getPatternGenerator();
            pg.setChordProgressionFromChordNames({chord.getChord().getSymbol()});
            for (auto progressionItem : pg.getChordProgression()) {
                progressionItem->lengthInBeats = chord.getDuration();
                progressionItem->chordName = chord.getChordname();
            }
            pg.mode = te::PatternGenerator::Mode::chords;
            pg.scaleRoot = 0;
            pg.octave = 7;
            pg.velocity = 30;
            pg.generatePattern();

        }
    }

    static inline bool nextExcerciseDateIsDue(tracktion_engine::AudioTrack *track) {
        if (!track->state.hasProperty(schwrrring::IDs::nextPracticeTime)) {
            return false;
        }

        PracticeTimes *practiceTimes = new PracticeTimes(
                track->state.getProperty(schwrrring::IDs::nextPracticeTime));
        long nextPractice = practiceTimes->times.getLast();

        if (nextPractice < getUnixTimeStamp()) {
            return true;
        } else {
            return false;
        }
    }

    class PrackticeTrackComparator {
    public:
        static int
        compareElements(juce::ValueTree firstTrack, juce::ValueTree secondTrack) {
            if (firstTrack.hasProperty(schwrrring::IDs::nextPracticeTime) &&
                !(secondTrack.hasProperty(schwrrring::IDs::nextPracticeTime))) {
                return 1;
            }
            if (!(firstTrack.hasProperty(schwrrring::IDs::nextPracticeTime)) &&
                secondTrack.hasProperty(schwrrring::IDs::nextPracticeTime)) {
                return -1;
            }
            if (!(firstTrack.hasProperty(schwrrring::IDs::nextPracticeTime)) &&
                !(secondTrack.hasProperty(schwrrring::IDs::nextPracticeTime))) {
                return 0;
            }
            auto firstPrac = firstTrack.getProperty(schwrrring::IDs::nextPracticeTime).toString();
            auto secondPrac = secondTrack.getProperty(schwrrring::IDs::nextPracticeTime).toString();

            PracticeTimes firstPracticeTimes{firstPrac};
            PracticeTimes secondPracticeTimes{secondPrac};

            return (firstPracticeTimes.times.getLast() < secondPracticeTimes.times.getLast()) ? -1
                                                                                              : ((secondPracticeTimes.times.getLast() <
                                                                                                  firstPracticeTimes.times.getLast())
                                                                                                 ? 1 : 0);
        }
    };

    static void sortTracksByPracticeTime(tracktion_engine::Edit &edit) {
        PrackticeTrackComparator prackticeTrackComparator;
        edit.state.sort(prackticeTrackComparator, nullptr, true);
    }

    static inline tracktion_engine::AudioTrack *getNextLesson(tracktion_engine::Edit &edit) {
        auto tracks = getAudioTracks(edit);
        sortTracksByPracticeTime(edit);
        return getAudioTracks(edit).getFirst();
    }

    static inline void colourClipsWithPracticeTimesInTheFutureYellow(tracktion_engine::Edit &edit) {
        auto tracks = getAudioTracks(edit);
        for (auto track : tracks) {
            if (track->state.hasProperty(schwrrring::IDs::nextPracticeTime)) {
                PracticeTimes practiceTimes{
                        track->state.getProperty(schwrrring::IDs::nextPracticeTime).toString().toStdString()};
                long retVal = getUnixTimeStamp();
                long nextPracticTime = practiceTimes.times.getLast();
                if (nextPracticTime > retVal) {

                    track->setColour(Colours::lightgrey);
                } else {
                    track->setColour(Colours::grey);
                }
            } else {
                track->setColour(Colours::grey);
            }
        }
    };

    static inline tracktion_engine::MidiNote *noteInSequence(
            juce::MidiKeyboardState &keyboardState,
            tracktion_engine::MidiList *midilist,
            const juce::MidiMessage &message) {

        keyboardState.processNextMidiEvent(message);
        auto midinotes = midilist->getNotes();
        for (tracktion_engine::MidiNote *midinote : midinotes) {
            if (keyboardState.isNoteOnForChannels(message.getChannel(), midinote->getNoteNumber())) {
                return midinote;
            }
        }
        return nullptr;
    }


    static inline bool deleteNoteFromSequenceAndConfirmDeletion(tracktion_engine::MidiNote *midiNote,
                                                                tracktion_engine::MidiList *midiList) {
        //    read and understand https://www.learncpp.com/cpp-tutorial/introduction-to-lambdas-anonymous-functions/
        if (midiNote != NULL) {
            midiList->removeNote(*midiNote, nullptr);

            return true;
        }
        return false;
    }

    static inline bool checkWhetherAllArpeggioNotesWerePlayed(tracktion_engine::MidiList *midiList) {
        return midiList->getNumNotes() == 0;
    }


    static inline bool handleArpeggio(juce::MidiKeyboardState &keyboardState,
                                      tracktion_engine::MidiClip *clip,
                                      const juce::MidiMessage &message,
                                      tracktion_engine::MidiList *midiList) {
        midiList->getNumNotes();
        Helpers::deleteNoteFromSequenceAndConfirmDeletion(Helpers::noteInSequence(keyboardState, midiList, message),
                                                          midiList);
        return midiList->getNumNotes() == 0;
    }

    static inline tracktion_engine::MidiClip *getNextClip(tracktion_engine::AudioTrack *track) {
        //        getFirstClip that has not
        auto tracks = track->getClips();
        auto clip = std::find_if(tracks.begin(), tracks.end(), [](tracktion_engine::Clip *clip) {
            return !(clip->state.hasProperty(schwrrring::IDs::isPlayed));
        });
        if (clip != tracks.end()) {
            return (tracktion_engine::MidiClip *) *clip;
        }
        return nullptr;
    }

    static inline bool checkChord(juce::MidiKeyboardState &keyboardState, tracktion_engine::MidiClip *clip,
                                  const juce::MidiMessage &message) {
        if (clip->hasValidSequence()) {
            keyboardState.processNextMidiEvent(message);
            tracktion_engine::MidiList &midilist = clip->getSequence();

            const Array<tracktion_engine::MidiNote *> &midinotes = midilist.getNotes();
            bool richtig_hund = true;
            for (tracktion_engine::MidiNote *midinote : midinotes) {
                if (!keyboardState.isNoteOnForChannels(message.getChannel(),
                                                       midinote->getNoteNumber())) {
                    richtig_hund = false;
                }
            }
            return richtig_hund;
        }
        return false;
    }

    static inline void colourClip(te::Clip *clip, Colour colour) {
        clip->setColour(colour);
    }

    static inline bool isExerciseCompleted(tracktion_engine::AudioTrack *track) {
        bool retVal = true;
        for (auto clip : track->getClips()) {
            if (!(clip->state.hasProperty(schwrrring::IDs::isPlayed))) {
                retVal = false;
            }
        }
        return retVal;
    }

    static inline void setIsPlayed(tracktion_engine::Clip *clip) {
        clip->state.setProperty(schwrrring::IDs::isPlayed, true, nullptr);
    }


    static inline void resetExerciseForNextPractice(tracktion_engine::AudioTrack *track) {
        for (auto clip : track->getClips()) {
            Helpers::colourClip(clip, Colours::red);
            clip->state.removeProperty(schwrrring::IDs::isPlayed, nullptr);
        }
    }


    static inline void setNextPracticeTime(tracktion_engine::Track *track) {
        track->state.removeProperty(schwrrring::IDs::isNewExercise, nullptr);
        if (!track->state.hasProperty(schwrrring::IDs::nextPracticeTime)) {
            PracticeTimes practiceTimes;
            long retVal = Helpers::getUnixTimeStamp();
            practiceTimes.times.add(retVal + 30L);
            track->state.setProperty(schwrrring::IDs::nextPracticeTime, (juce::String) practiceTimes.toString(),
                                     nullptr);
        } else {
            PracticeTimes practiceTimes{track->state.getProperty(schwrrring::IDs::nextPracticeTime)};
            long lastPracticeTime = practiceTimes.times.getLast();
            practiceTimes.times.add(Helpers::getUnixTimeStamp() + 30L);
            track->state.setProperty(schwrrring::IDs::nextPracticeTime, practiceTimes.toString(),
                                     nullptr);
        }
    }

}


//==============================================================================
class FlaggedAsyncUpdater : public AsyncUpdater {
public:
    //==============================================================================
    void markAndUpdate(bool &flag) {
        flag = true;
        triggerAsyncUpdate();
    }

    bool compareAndReset(bool &flag) noexcept {
        if (!flag)
            return false;

        flag = false;
        return true;
    }
};

//==============================================================================
struct Thumbnail : public Component {
    Thumbnail(te::TransportControl &tc)
            : transport(tc) {
        cursorUpdater.setCallback([this] {
            updateCursorPosition();

            if (smartThumbnail.isGeneratingProxy() || smartThumbnail.isOutOfDate())
                repaint();
        });
        cursor.setFill(findColour(Label::textColourId));
        addAndMakeVisible(cursor);
    }

    void setFile(const te::AudioFile &file) {
        smartThumbnail.setNewFile(file);
        cursorUpdater.startTimerHz(25);
        repaint();
    }

    void paint(Graphics &g) override {
        auto r = getLocalBounds();
        const auto colour = findColour(Label::textColourId);

        if (smartThumbnail.isGeneratingProxy()) {
            g.setColour(colour.withMultipliedBrightness(0.9f));
            g.drawText("Creating proxy: " + juce::String(roundToInt(smartThumbnail.getProxyProgress() * 100.0f)) + "%",
                       r, Justification::centred);

        } else {
            const float brightness = smartThumbnail.isOutOfDate() ? 0.4f : 0.66f;
            g.setColour(colour.withMultipliedBrightness(brightness));
            smartThumbnail.drawChannels(g, r, true, {0.0, smartThumbnail.getTotalLength()}, 1.0f);
        }
    }

    void mouseDown(const MouseEvent &e) override {
        transport.setUserDragging(true);
        mouseDrag(e);
    }

    void mouseDrag(const MouseEvent &e) override {
        jassert (getWidth() > 0);
        const float proportion = e.position.x / getWidth();
        transport.position = proportion * transport.getLoopRange().getLength();
    }

    void mouseUp(const MouseEvent &) override {
        transport.setUserDragging(false);
    }

private:
    te::TransportControl &transport;
    te::SmartThumbnail smartThumbnail{transport.engine, te::AudioFile(transport.engine), *this, nullptr};
    DrawableRectangle cursor;
    te::LambdaTimer cursorUpdater;

    void updateCursorPosition() {
        const double loopLength = transport.getLoopRange().getLength();
        const double proportion = loopLength == 0.0 ? 0.0 : transport.getCurrentPosition() / loopLength;

        auto r = getLocalBounds().toFloat();
        const float x = r.getWidth() * float(proportion);
        cursor.setRectangle(r.withWidth(2.0f).withX(x));
    }
};
