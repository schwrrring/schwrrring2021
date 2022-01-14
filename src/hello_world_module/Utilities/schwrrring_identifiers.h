//
// Created by malte on 14.01.22.
//


/*
    ,--.                     ,--.     ,--.  ,--.
  ,-'  '-.,--.--.,--,--.,---.|  |,-.,-'  '-.`--' ,---. ,--,--,      Copyright 2018
  '-.  .-'|  .--' ,-.  | .--'|     /'-.  .-',--.| .-. ||      \   Tracktion Software
    |  |  |  |  \ '-'  \ `--.|  \  \  |  |  |  |' '-' '|  ||  |       Corporation
    `---' `--'   `--`--'`---'`--'`--' `---' `--' `---' `--''--'    www.tracktion.com

    Tracktion Engine uses a GPL/commercial licence - see LICENCE.md for details.
*/

namespace schwrrring
{

    namespace IDs
    {
#define DECLARE_ID(name)  const juce::Identifier name (#name);
        DECLARE_ID (nextPracticeTime)
        DECLARE_ID (isPlayed)
        DECLARE_ID (isNewExercise)


#undef DECLARE_ID
    }

} // namespace tracktion_engine


