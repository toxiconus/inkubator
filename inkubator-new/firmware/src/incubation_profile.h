// incubation_profile.h
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <vector>

struct IncubationDay {
    int day;
    float shell_temp;
    float air_temp_target;
    float humidity_set;
    float humidity_min;
    float humidity_max;
    bool turning;
    bool misting;
    String sensitivity;
    String status_color;
    String notes;
};

struct SpeciesProfile {
    String id;
    String common_name;
    int incubation_days;
    String description;
    String reference;
    int turning_frequency_per_day;
    std::map<String, JsonArray> critical_phases;
    std::vector<IncubationDay> days;
    
    SpeciesProfile() : incubation_days(0) {}
    ~SpeciesProfile() { free(); }
    void free() { days.clear(); incubation_days = 0; }
};

struct IncubationProfiles {
    std::map<String, SpeciesProfile> species_map;
    std::map<String, String> sensitivity_colors;
};

bool loadAllProfiles(const char* path, IncubationProfiles& profiles);
const SpeciesProfile* getSpeciesProfile(const IncubationProfiles& profiles, const String& species_id);
IncubationDay getDayForSpecies(const IncubationProfiles& profiles, const String& species_id, int day);
void freeProfiles(IncubationProfiles& profiles);