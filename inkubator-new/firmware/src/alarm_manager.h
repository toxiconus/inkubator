// alarm_manager.h
#pragma once

#include "controller.h"

void check_alarms(const SensorData& data);

// Stan alarmu dla API: kod (np. "temp_high", pusty gdy brak), komunikat i czas rozpoczęcia (millis)
const char* alarm_code();
const char* alarm_message();
unsigned long alarm_since_ms();
