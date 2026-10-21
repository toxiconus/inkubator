#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

bool load_species_profiles();
JsonVariant find_species_profile(const String& species_id);
JsonVariant get_active_phase(JsonVariant profile, int current_day);
String serialize_species_profiles();
String api_species_profiles_active();

struct ChamberAggregation {
  float target_temp;
  float temp_min;
  float temp_max;
  float humidity_min;
  float humidity_max;
  bool valid;
};
