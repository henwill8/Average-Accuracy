#include "../include/main.hpp"
#include "../extern/beatsaber-hook/shared/customui/customui.hpp"
#include "../extern/beatsaber-hook/shared/config/config-utils.hpp"

#include <math.h>
#include <cmath>
#include <iostream>

bool runScores = false;

int num = 0;
int num2 = 0;
int sum = 0;
int sum2 = 0;
int sumAcc = 0;
float average = 0.0f;
float averageAcc = 0.0f;
float sessionAverage = 0.0f; 

char averageFinal[40] = {0};
char averageAccFinal[40] = {0};
char averageSessionFinal[40] = {0};

int songs = 0;

bool advancedHud;

CustomUI::TextObject averageText;

MAKE_HOOK_OFFSETLESS(RawScore, void, Il2CppObject* noteCutInfo, int* beforeCutRawScore, int* afterCutRawScore, int* cutDistanceRawScore) {
    RawScore(noteCutInfo, beforeCutRawScore, afterCutRawScore, cutDistanceRawScore);

    if(runScores) {
        int beforeCut = *beforeCutRawScore;
        int afterCut = *afterCutRawScore;
        int cutDist = *cutDistanceRawScore;

        int totalScore = beforeCut+afterCut+cutDist;

        sumAcc += cutDist;
        sum += totalScore;
        sum2 += totalScore;
        num++;
        num2++;

        averageAcc = float(sumAcc) / float(num);
        average = float(sum) / float(num);
        sessionAverage = float(sum2) / float(num2);

        log(DEBUG, "%f %f %f", averageAcc, average, sessionAverage);

        sprintf(averageFinal, "%.2f", average);
        sprintf(averageAccFinal, "%.2f", averageAcc);
        sprintf(averageSessionFinal, "%.2f", sessionAverage);
        il2cpp_utils::RunMethod(averageText.textMesh, "set_text", il2cpp_utils::createcsstr(std::string("Average Score : ") + averageFinal + std::string("\nAverage Accuracy : ") + averageAccFinal + std::string("\nAverage Session Score : ") + averageSessionFinal));
        runScores = false;
    }
}

MAKE_HOOK_OFFSETLESS(FinishScore, void, Il2CppObject* self, Il2CppObject* swingRatingCounter) {
    runScores = true;

    FinishScore(self, swingRatingCounter);
}

MAKE_HOOK_OFFSETLESS(SongStart, void, Il2CppObject* self, Il2CppObject* difficultyBeatmap, Il2CppObject* overrideEnvironmentSettings, Il2CppObject* overrideColorScheme, Il2CppObject* gameplayModifiers, Il2CppObject* playerSpecificSettings, Il2CppObject* practiceSettings, Il2CppString* backButtonText, bool useTestNoteCutSoundEffects) {
    il2cpp_utils::RunMethod(&advancedHud, playerSpecificSettings, "get_advancedHud");
    
    SongStart(self, difficultyBeatmap, overrideEnvironmentSettings, overrideColorScheme, gameplayModifiers, playerSpecificSettings, practiceSettings, backButtonText, useTestNoteCutSoundEffects);
}

MAKE_HOOK_OFFSETLESS(Start, void, Il2CppObject* self) {
    songs++;
    
    if(advancedHud) {
        Il2CppObject* score = il2cpp_utils::GetFieldValue(self, "_scoreText");
        Il2CppObject* scoreTransform;
        Il2CppObject* scoreParent;

        il2cpp_utils::RunMethod(&scoreTransform, score, "get_transform");
        il2cpp_utils::RunMethod(&scoreParent, scoreTransform, "GetParent");

        averageText.text = "Average Score : 0\nAverage Accuracy : 0\nAverage Session Score : " + std::string(averageSessionFinal);
        averageText.fontSize = 12.0F;        
        averageText.parentTransform = scoreParent;
        averageText.sizeDelta = {-615, 220};
        averageText.create();
    }

    if(songs > 1) {
        num = 0;
        sumAcc = 0;
        sum = 0;
    }
    
    Start(self);
}

extern "C" void load() {
    log(INFO, "Installing hooks...");
    INSTALL_HOOK_OFFSETLESS(RawScore, il2cpp_utils::FindMethodUnsafe("", "ScoreModel", "RawScoreWithoutMultiplier", 4));
    INSTALL_HOOK_OFFSETLESS(FinishScore, il2cpp_utils::FindMethodUnsafe("", "CutScoreBuffer", "HandleSwingRatingCounterDidFinishEvent", 1));
    INSTALL_HOOK_OFFSETLESS(SongStart, il2cpp_utils::FindMethodUnsafe("", "StandardLevelScenesTransitionSetupDataSO", "Init", 8));
    INSTALL_HOOK_OFFSETLESS(Start, il2cpp_utils::FindMethodUnsafe("", "ScoreUIController", "Start", 0));
    log(INFO, "Installed all hooks!");
}