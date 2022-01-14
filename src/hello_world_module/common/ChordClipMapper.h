//
// Created by Malte Hildebrand on 03.12.21.
//

#ifndef EASYEARS_CHORDCLIPMAPPER_H
#define EASYEARS_CHORDCLIPMAPPER_H

class ChordClipMapper {

public:
    ChordClipMapper(mx::api::ChordKind kindText, mx::api::Step rootStep, int rootAlter, double duration, int divisions,
                    int measure, int tickTimePosition)
            : kindText{kindText}, rootStep{rootStep}, rootAlter{rootAlter}, duration{duration},
              divisions{divisions}, measure{measure}, tickTimePosition{tickTimePosition} {
        using namespace mx::api;
        using namespace mx::core;
        tracktion_engine::Chord::ChordType chordType = tracktion_engine::Chord::ChordType::augmentedDominantNinthChord;
        chord = tracktion_engine::Chord(getXmlChordType(kindText));
    }

    tracktion_engine::Chord getChord() {
        return chord;
    }

    tracktion_engine::Chord::ChordType getXmlChordType(mx::api::ChordKind chordKind) {

        if (chordKind == mx::api::ChordKind::augmented) { return tracktion_engine::Chord::ChordType::augmentedTriad; };

        if (chordKind ==
            mx::api::ChordKind::augmentedSeventh) { return tracktion_engine::Chord::ChordType::augmentedSeventhChord; };
        if (chordKind ==
            mx::api::ChordKind::diminished) { return tracktion_engine::Chord::ChordType::diminishedTriad; };
        if (chordKind ==
            mx::api::ChordKind::diminishedSeventh) { return tracktion_engine::Chord::ChordType::diminishedSeventhChord; };
        if (chordKind ==
            mx::api::ChordKind::dominant) { return tracktion_engine::Chord::ChordType::dominatSeventhChord; };
//        if (string == "dominant-11th") { return tracktion_engine::Chord::ChordType::do; };
        if (chordKind ==
            mx::api::ChordKind::dominantNinth) { return tracktion_engine::Chord::ChordType::dominantNinthChord; };
        if (chordKind ==
            mx::api::ChordKind::halfDiminished) { return tracktion_engine::Chord::ChordType::halfDiminishedSeventhChord; };
        if (chordKind == mx::api::ChordKind::major) { return tracktion_engine::Chord::ChordType::majorTriad; };
        if (chordKind ==
            mx::api::ChordKind::majorNinth) { return tracktion_engine::Chord::ChordType::majorNinthChord; };
//        if (string == "major-13th") { return tracktion_engine::Chord::ChordType::ma;};
        if (chordKind ==
            mx::api::ChordKind::majorSeventh) { return tracktion_engine::Chord::ChordType::majorSeventhChord; };
        if (chordKind == mx::api::ChordKind::minor) { return tracktion_engine::Chord::ChordType::minorTriad; };
//        if (string == "minor-ninth") { return tracktion_engine::Chord::ChordType::minorDominantNinthChord;};
        if (chordKind ==
            mx::api::ChordKind::minorSeventh) { return tracktion_engine::Chord::ChordType::minorSeventhChord; };
    }

    double getDuration() {
        return duration;
//        return std::stold(duration.toStdString()) / std::stold(divisions.toStdString());
    }

    void setDuration(double duration) {
        this->duration = duration;
    }

    String getChordname() {
        String chordname1 = stepEnumToString(rootStep) + getAlterSign() + chord.getSymbol();
        return chordname1;
    }

    String getAlterSign() {

        switch (rootAlter) {
            case 0:
                return "";
                break;
            case -1:
                return "b";
                break;
            case 1:
                return "#";
                break;
        }
    }

    int getMeasure() const {
        return this->measure;
    }

//    todo: malte write test
public:
    tracktion_engine::EditTimeRange calculateBarsAndBeatsForChord(tracktion_engine::Edit &edit) {
       return tracktion_engine::EditTimeRange(edit.tempoSequence.barsBeatsToTime(
                                                {measure, (double) tickTimePosition/divisions}),
                                        edit.tempoSequence.barsBeatsToTime(
                                                {measure, (double) tickTimePosition/divisions + duration}));
    }

    String stepEnumToString(mx::api::Step step) {
        switch (step) {
            case mx::api::Step::c:
                return "C";
            case mx::api::Step::d:
                return "D";
            case mx::api::Step::e:
                return "E";
            case mx::api::Step::f:
                return "F";
            case mx::api::Step::g:
                return "G";
            case mx::api::Step::a:
                return "A";
            case mx::api::Step::b:
                return "B";
//            cacse todo add unspecified!
        }
    }

    int tickTimePosition;

    int getTickTimePosition() const {
        return tickTimePosition;
    }

private:
    mx::api::Step rootStep;
    int rootAlter;
    double duration;
    int divisions;
    tracktion_engine::Chord chord;
    int measure;
    mx::api::ChordKind kindText;
    std::vector<String> stepsByEnum{"one", "two", "three"};
};

#endif //EASYEARS_CHORDCLIPMAPPER_H
