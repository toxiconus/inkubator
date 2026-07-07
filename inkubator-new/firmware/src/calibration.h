#pragma once

struct CalibrationState {
    bool running = false;
    unsigned long startMs = 0;
    unsigned long elapsedMs = 0;
    int pwmPercent = 60;
    String phase = "0→MAX_P";
    unsigned long seqStartMs = 0;
    int seqCount = 0;
    int seqMax = 10;
    bool seqRunning = false;
};

extern CalibrationState calState;
