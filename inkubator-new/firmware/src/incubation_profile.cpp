// incubation_profile.cpp
#include "incubation_profile.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

bool loadAllProfiles(const char* path, IncubationProfiles& profiles) {
    for (auto& pair : profiles.species_map) pair.second.free();
    profiles.species_map.clear();
    profiles.sensitivity_colors.clear();

    File file = LittleFS.open(path, "r");
    if (!file) return false;

    String jsonStr;
    while (file.available()) jsonStr += (char)file.read();
    file.close();

    JsonDocument doc;
    if (deserializeJson(doc, jsonStr)) return false;

    JsonObject colors = doc["sensitivity_colors"].as<JsonObject>();
    if (colors) {
        for (JsonPair kv : colors) {
            profiles.sensitivity_colors[kv.key().c_str()] = kv.value().as<String>();
        }
    }

    JsonObject speciesObj = doc["species"].as<JsonObject>();
    if (!speciesObj) return false;

    for (JsonPair kv : speciesObj) {
        String speciesKey = kv.key().c_str();
        JsonObject sp = kv.value().as<JsonObject>();
        if (!sp) continue;

        SpeciesProfile profile;
        profile.id = sp["id"] | speciesKey;
        profile.common_name = sp["common_name"] | speciesKey;
        profile.incubation_days = sp["incubation_days"] | 21;
        profile.description = sp["description"] | "";
        profile.reference = sp["reference"] | "";
        profile.turning_frequency_per_day = sp["turning_frequency_per_day"] | 4;

        JsonArray daysArray = sp["day_by_day"].as<JsonArray>();
        if (!daysArray) continue;

        profile.days.resize(profile.incubation_days);
        for (int i = 0; i < profile.incubation_days && i < (int)daysArray.size(); i++) {
            JsonObject dayObj = daysArray[i];
            profile.days[i].day = dayObj["day"] | (i + 1);
            profile.days[i].shell_temp = dayObj["shell_temp"] | 37.8f;
            profile.days[i].air_temp_target = dayObj["air_temp_target"] | 37.5f;
            profile.days[i].humidity_set = dayObj["humidity_set"] | 50;
            if (dayObj["humidity_range"].is<JsonArray>()) {
                JsonArray range = dayObj["humidity_range"].as<JsonArray>();
                profile.days[i].humidity_min = range[0] | 40;
                profile.days[i].humidity_max = range[1] | 60;
            }
            profile.days[i].turning = dayObj["turning"] | true;
            profile.days[i].misting = dayObj["misting"] | false;
            profile.days[i].sensitivity = dayObj["sensitivity"] | "MEDIUM";
            profile.days[i].status_color = dayObj["status_color"] | "#70d070";
            profile.days[i].notes = dayObj["notes"] | "";
        }
        profiles.species_map[speciesKey] = profile;
    }
    return true;
}

const SpeciesProfile* getSpeciesProfile(const IncubationProfiles& profiles, const String& species_id) {
    auto it = profiles.species_map.find(species_id);
    if (it != profiles.species_map.end()) return &it->second;
    return nullptr;
}

IncubationDay getDayForSpecies(const IncubationProfiles& profiles, const String& species_id, int day) {
    IncubationDay empty;
    empty.day = day;
    empty.shell_temp = 37.8;
    empty.air_temp_target = 37.5;
    empty.humidity_set = 50;
    empty.humidity_min = 40;
    empty.humidity_max = 60;
    empty.turning = true;
    empty.misting = false;
    empty.sensitivity = "MEDIUM";
    empty.status_color = "#70d070";
    empty.notes = "";
    
    const SpeciesProfile* sp = getSpeciesProfile(profiles, species_id);
    if (!sp || sp->days.empty() || day < 1 || day > sp->incubation_days) return empty;
    return sp->days[day - 1];
}

void freeProfiles(IncubationProfiles& profiles) {
    for (auto& pair : profiles.species_map) pair.second.free();
    profiles.species_map.clear();
    profiles.sensitivity_colors.clear();
}